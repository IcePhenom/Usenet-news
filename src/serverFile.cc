#include "server.h"
#include "connection.h"
#include "connectionclosedexception.h"
#include "protocol.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;
using namespace client_server;
using protocol::Protocol;

const string DB = "DB";

void writeNumber(int value, Connection* conn) {
    conn->write((value >> 24) & 0xFF);
    conn->write((value >> 16) & 0xFF);
    conn->write((value >> 8)  & 0xFF);
    conn->write(value & 0xFF);
}

int readNumber(Connection* conn) {
    unsigned char byte1 = conn->read();
    unsigned char byte2 = conn->read();
    unsigned char byte3 = conn->read();
    unsigned char byte4 = conn->read();
    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

void writeString(const string& s, Connection* conn) {
    for (size_t i = 0; i < s.size(); ++i)
        conn->write(s[i]);
}

string readString(int N, Connection* conn) {
    string s;
    for(int i = 0; i < N; ++i)
        s += conn->read();
    return s;
}

void writeByte(int value, Connection* conn){
    conn->write(value);
}

int readByte(Connection* conn){
    return conn->read();
}

int nbrNG(){
    DIR *dir;
    struct dirent *ent;
    int count = 0;
    dir = opendir(DB.c_str());
    while((ent = readdir(dir)) != NULL){
        string name = ent->d_name;
        if(name != "." && name != "..")
            count++;
    }
    closedir(dir);
    return count;
}

void listNGFile(Connection* conn){
    DIR *dir;
    struct dirent *ent;
    dir = opendir(DB.c_str());
    string id, name, title;
    vector<string> l;
    while((ent = readdir(dir)) != NULL){
        name = ent->d_name;
        if(name != "." && name != "..")
            l.push_back(name);
    }
    sort(l.begin(), l.end());

    for(size_t q = 0; q < l.size(); ++q){
            istringstream iss(l[q]);
            getline(iss,id,'+');
            getline(iss,title);
            writeByte(Protocol::PAR_NUM, conn);
            writeNumber(atoi(id.c_str()), conn);
            writeByte(Protocol::PAR_STRING, conn);
            writeNumber(title.length(), conn);
            writeString(title, conn);
    }
}

void listNG(Connection* conn){
    if(readByte(conn)==Protocol::COM_END){
        writeByte(Protocol::ANS_LIST_NG, conn);
        writeByte(Protocol::PAR_NUM, conn);
        writeNumber(nbrNG(), conn);
        listNGFile(conn);
        writeByte(Protocol::ANS_END, conn);
    }
}

bool checkExisting(string s){
    DIR *dir;
    struct dirent *ent;
    dir = opendir(DB.c_str());
    string name;
    while((ent = readdir(dir)) != NULL){
        name = ent->d_name;
        if(name.find(s) != string::npos)
            return true;
    }
    return false;
}

void createNG(Connection* conn){
    if(readByte(conn)==Protocol::PAR_STRING){
        string s = readString(readNumber(conn), conn);
        if(readByte(conn)==Protocol::COM_END){
            writeByte(Protocol::ANS_CREATE_NG, conn);
            if(!checkExisting(s)){
                string c = DB + "/" + to_string(nbrNG()+1) + "+" + s;
                mkdir(c.c_str(), 0777);
                writeByte(Protocol::ANS_ACK, conn);
            } else {
                writeByte(Protocol::ANS_NAK, conn);
                writeByte(Protocol::ERR_NG_ALREADY_EXISTS, conn);
            }
            writeByte(Protocol::ANS_END, conn);
        }
    }
}

bool findFile(int nbr, string folder, string &file){
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(folder.c_str())) != NULL){
        int i;
        while((ent = readdir(dir)) != NULL){
            sscanf(ent->d_name, "%d", &i);
            if(nbr == i){
                file = ent->d_name;
                return true;
            }
        }
    }
    return false;
}

bool findFolder(int nbr, string &folder){
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(DB.c_str())) != NULL){
        int n;
        while((ent = readdir(dir)) != NULL){
            sscanf(ent->d_name, "%d", &n);
            if(nbr == n){
                folder = DB + "/" + ent->d_name;
                return true;
            }
        }
    }
    return false;
}

bool deleteIfExisting(int i){
    string folder;
    if(findFolder(i, folder)){
        DIR *dir;
        struct dirent *ent;
        if((dir = opendir(folder.c_str())) != NULL){
            while((ent = readdir(dir)) != NULL){
                remove((folder + "/" + ent->d_name).c_str());
            }
        }
        closedir(dir);
        rmdir(folder.c_str());
        return true;
    }
    return false;
}

void deleteNG(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int del = readNumber(conn);
        if(readByte(conn)==Protocol::COM_END){
            writeByte(Protocol::ANS_DELETE_NG, conn);
            if(deleteIfExisting(del)){
                writeByte(Protocol::ANS_ACK, conn);
            } else {
                writeByte(Protocol::ANS_NAK, conn);
                writeByte(Protocol::ERR_NG_DOES_NOT_EXIST, conn);
            }
            writeByte(Protocol::ANS_END, conn);
        }
    }
}

int nbrArt(string folder){
    DIR *dir;
    struct dirent *ent;
    int count = 0;
    dir = opendir(folder.c_str());
    while((ent = readdir(dir)) != NULL){
        string name = ent->d_name;
        if(name != "." && name != ".."){
            ++count;
        }
    }
    closedir(dir);
    return count;
}

void listART(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int nbr = readNumber(conn);
        if(readByte(conn)==Protocol::COM_END){
            writeByte(Protocol::ANS_LIST_ART, conn);
            string folder;
            if(findFolder(nbr, folder)){
                DIR *dir;
                struct dirent *ent;
                if((dir = opendir(folder.c_str()))!= NULL){
                    writeByte(Protocol::ANS_ACK, conn);
                    writeByte(Protocol::PAR_NUM, conn);
                    writeNumber(nbrArt(folder), conn);
                    string name, id, title;
                    vector<string> l;
                    while((ent = readdir(dir)) != NULL){
                        name = ent->d_name;
                        if(name != "." && name != "..")
                            l.push_back(name);
                    }
                    sort(l.begin(), l.end());

                    for(size_t q = 0; q < l.size(); ++q){
                        istringstream iss(l[q]);
                        getline(iss,id,'+');
                        getline(iss,title,'+');
                        writeByte(Protocol::PAR_NUM, conn);
                        writeNumber(atoi(id.c_str()), conn);
                        writeByte(Protocol::PAR_STRING, conn);
                        writeNumber(title.length(), conn);
                        writeString(title, conn);
                    }
                    writeByte(Protocol::ANS_END, conn);
                    return;
                }
            }
            writeByte(Protocol::ANS_NAK, conn);
            writeByte(Protocol::ERR_NG_DOES_NOT_EXIST, conn);
            writeByte(Protocol::ANS_END, conn);
        }
    }
}

void createART(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int ng = readNumber(conn);
        string title, author, text;
        if(readByte(conn)==Protocol::PAR_STRING)
            title = readString(readNumber(conn), conn);
        if(readByte(conn)==Protocol::PAR_STRING)
            author = readString(readNumber(conn), conn);
        if(readByte(conn)==Protocol::PAR_STRING)
            text = readString(readNumber(conn), conn);

        if(readByte(conn)==Protocol::COM_END){
            writeByte(Protocol::ANS_CREATE_ART, conn);
            string folder;
            if(findFolder(ng, folder)){
                int unID = nbrArt(folder) + 1;
                string file = folder + "/" + to_string(unID) + "+" + title + "+" + author;
                ofstream out(file);
                out << text;
                out.close();
                writeByte(Protocol::ANS_ACK, conn);
                writeByte(Protocol::ANS_END, conn);
                return;
            }
        }
        writeByte(Protocol::ANS_NAK, conn);
        writeByte(Protocol::ERR_NG_DOES_NOT_EXIST, conn);
        writeByte(Protocol::ANS_END, conn);
    }
}

void deleteART(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int ng = readNumber(conn);
        if(readByte(conn)==Protocol::PAR_NUM){
            int art = readNumber(conn);
            if(readByte(conn)==Protocol::COM_END){
                writeByte(Protocol::ANS_DELETE_ART, conn);
                string folder;
                if(findFolder(ng, folder)){
                    string file;
                    if(findFile(art, folder, file)){
                        remove((folder + "/" + file).c_str());
                        writeByte(Protocol::ANS_ACK, conn);
                        writeByte(Protocol::ANS_END, conn);
                        return;
                    }
                    writeByte(Protocol::ANS_NAK, conn);
                    writeByte(Protocol::ERR_ART_DOES_NOT_EXIST, conn);
                    writeByte(Protocol::ANS_END, conn);
                    return;
                }
                writeByte(Protocol::ANS_NAK, conn);
                writeByte(Protocol::ERR_NG_DOES_NOT_EXIST, conn);
                writeByte(Protocol::ANS_END, conn);
            }
        }
    }
}

string readFileString(string file){
    stringstream ret;
    ifstream in(file.c_str());
    ret << in.rdbuf();
    in.close();
    return ret.str();
}

void getART(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int ng = readNumber(conn);
        if(readByte(conn)==Protocol::PAR_NUM){
            int art = readNumber(conn);
            if(readByte(conn)==Protocol::COM_END){
                writeByte(Protocol::ANS_GET_ART, conn);
                string folder;
                if(findFolder(ng, folder)){
                    string file;
                    if(findFile(art, folder, file)){
                        string id, title, author;
                        istringstream iss(file);
                        getline(iss,id,'+');
                        getline(iss,title,'+');
                        getline(iss,author);

                        string text = readFileString(folder + "/" + file);

                        writeByte(Protocol::ANS_ACK, conn);
                        writeByte(Protocol::PAR_STRING, conn);
                        writeNumber(title.size(), conn);
                        writeString(title, conn);

                        writeByte(Protocol::PAR_STRING, conn);
                        writeNumber(author.size(), conn);
                        writeString(author, conn);

                        writeByte(Protocol::PAR_STRING, conn);
                        writeNumber(text.size(), conn);
                        writeString(text, conn);
                        writeByte(Protocol::ANS_END, conn);
                        return;
                    }
                    writeByte(Protocol::ANS_NAK, conn);
                    writeByte(Protocol::ERR_ART_DOES_NOT_EXIST, conn);
                    writeByte(Protocol::ANS_END, conn);
                    return;
                }
                writeByte(Protocol::ANS_NAK, conn);
                writeByte(Protocol::ERR_NG_DOES_NOT_EXIST, conn);
                writeByte(Protocol::ANS_END, conn);
            }
        }
    }
}

int main(int argc, char* argv[]){
    if (argc != 2) {
        cerr << "Usage: server port-number" << endl;
        exit(1);
    }

    Server server(atoi(argv[1]));
    if (! server.isReady()) {
        cerr << "Server initialization error" << endl;
        exit(1);
    }

    mkdir(DB.c_str(), 0777);
    cout << "Server started" << endl;
    while (true) {
        Connection* conn = server.waitForActivity();
        if (conn != 0) {
            try {
                int nbr = readByte(conn);
                switch(nbr){
                    case Protocol::COM_LIST_NG: listNG(conn); break;
                    case Protocol::COM_CREATE_NG: createNG(conn); break;
                    case Protocol::COM_DELETE_NG: deleteNG(conn); break;
                    case Protocol::COM_LIST_ART: listART(conn); break;
                    case Protocol::COM_CREATE_ART: createART(conn); break;
                    case Protocol::COM_DELETE_ART: deleteART(conn); break;
                    case Protocol::COM_GET_ART: getART(conn); break;
                }
            }
            catch (ConnectionClosedException&) {
                server.deregisterConnection(conn);
                delete conn;
                cout << "Client closed connection" << endl;
            }
        }
        else {
            server.registerConnection(new Connection);
            cout << "New client connects" << endl;
        }
    }
}
