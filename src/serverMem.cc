#include "server.h"
#include "connection.h"
#include "connectionclosedexception.h"
#include "protocol.h"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace client_server;
using protocol::Protocol;

class article{
public:
    int id;
    string title, author, text;
    article(int i, string t, string a, string s): id(i), title(t), author(a), text(s){}
};

class newsGroup{
public:
    int id;
    int artUNIQE=0;
    string name;
    vector<article> v;
    newsGroup(int i, string s): id(i), name(s){}
};

vector<newsGroup> vec;
int ngUNIQE = 0;

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

void listNG(Connection* conn){
    if(readByte(conn)==Protocol::COM_END){
        writeByte(Protocol::ANS_LIST_NG, conn);
        writeByte(Protocol::PAR_NUM, conn);
        writeNumber(vec.size(), conn);
        vector<newsGroup>::iterator it = vec.begin();
        while(it != vec.end()){
            writeByte(Protocol::PAR_NUM, conn);
            writeNumber(it->id, conn);
            writeByte(Protocol::PAR_STRING, conn);
            writeNumber(it->name.length(), conn);
            writeString(it->name, conn);
            ++it;
        }
        writeByte(Protocol::ANS_END, conn);
    }
}

void createNG(Connection* conn){
    if(readByte(conn)==Protocol::PAR_STRING){
        string s = readString(readNumber(conn), conn);
        if(readByte(conn)==Protocol::COM_END){
            writeByte(Protocol::ANS_CREATE_NG, conn);
            vector<newsGroup>::iterator it = find_if(vec.begin(), vec.end(),
                [s](const newsGroup &a){return a.name.compare(s)==0;});
            if(it == vec.end()){
                vec.push_back(newsGroup(ngUNIQE, s));
                ++ngUNIQE;
                writeByte(Protocol::ANS_ACK, conn);
            } else {
                writeByte(Protocol::ANS_NAK, conn);
                writeByte(Protocol::ERR_NG_ALREADY_EXISTS, conn);
            }
            writeByte(Protocol::ANS_END, conn);
        }
    }
}

void deleteNG(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int i = readNumber(conn);
        if(readByte(conn)==Protocol::COM_END){
            writeByte(Protocol::ANS_DELETE_NG, conn);
            vector<newsGroup>::iterator it = find_if(vec.begin(), vec.end(),
                [i](const newsGroup &a){return a.id == i;});
            if(it != vec.end()){
                it->v.clear();
                vec.erase(it);
                writeByte(Protocol::ANS_ACK, conn);
            } else {
                writeByte(Protocol::ANS_NAK, conn);
                writeByte(Protocol::ERR_NG_DOES_NOT_EXIST, conn);
            }
            writeByte(Protocol::ANS_END, conn);
        }
    }
}

void listART(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int nbr = readNumber(conn);
        if(readByte(conn)==Protocol::COM_END){
            writeByte(Protocol::ANS_LIST_ART, conn);
            vector<newsGroup>::iterator it = find_if(vec.begin(), vec.end(),
                [nbr](const newsGroup &a){return a.id == nbr;});
            if(it != vec.end()){
                writeByte(Protocol::ANS_ACK, conn);
                writeByte(Protocol::PAR_NUM, conn);
                writeNumber(it->v.size(), conn);
                vector<article>::iterator itr = it->v.begin();
                while(itr != it->v.end()){
                    writeByte(Protocol::PAR_NUM, conn);
                    writeNumber(itr->id, conn);
                    writeByte(Protocol::PAR_STRING, conn);
                    writeNumber(itr->title.size(), conn);
                    writeString(itr->title, conn);
                    ++itr;
                }
                writeByte(Protocol::ANS_END, conn);
                return;
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
            vector<newsGroup>::iterator it = find_if(vec.begin(), vec.end(),
                [ng](const newsGroup &a){return a.id == ng;});
            if(it != vec.end()){
                it->v.push_back(article(it->artUNIQE, title, author, text));
                it->artUNIQE++;
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
                vector<newsGroup>::iterator it = find_if(vec.begin(), vec.end(),
                    [ng](const newsGroup &a){return a.id == ng;});
                if(it != vec.end()){
                    vector<article>::iterator itr = find_if(it->v.begin(), it->v.end(),
                        [art](const article &a){return a.id == art;});
                    if(itr != it->v.end()){
                        it->v.erase(itr);
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

void getART(Connection* conn){
    if(readByte(conn)==Protocol::PAR_NUM){
        int ng = readNumber(conn);
        if(readByte(conn)==Protocol::PAR_NUM){
            int art = readNumber(conn);
            if(readByte(conn)==Protocol::COM_END){
                writeByte(Protocol::ANS_GET_ART, conn);
                vector<newsGroup>::iterator it = find_if(vec.begin(), vec.end(),
                    [ng](const newsGroup &a){return a.id == ng;});
                if(it != vec.end()){
                    vector<article>::iterator itr = find_if(it->v.begin(), it->v.end(),
                        [art](const article &a){return a.id == art;});
                    if(itr != it->v.end()){
                        writeByte(Protocol::ANS_ACK, conn);
                        writeByte(Protocol::PAR_STRING, conn);
                        writeNumber(itr->title.size(), conn);
                        writeString(itr->title, conn);

                        writeByte(Protocol::PAR_STRING, conn);
                        writeNumber(itr->author.size(), conn);
                        writeString(itr->author, conn);

                        writeByte(Protocol::PAR_STRING, conn);
                        writeNumber(itr->text.size(), conn);
                        writeString(itr->text, conn);
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
