#include "connection.h"
#include "connectionclosedexception.h"
#include "protocol.h"
#include "message.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <limits>
#include <vector>

using namespace std;
using namespace client_server;
using namespace message;
using protocol::Protocol;

void done(){
    cout << "\nDONE\n\nEnter choice: ";
}

void listNG(Connection& conn){
    cout << "Listing NewsGroups" << endl;

    vector<pair<int,string> > s = listNGClient(conn);

    if(s.size() > 0)
        for (size_t i = 0; i < s.size(); ++i)
            cout << s[i].first << " " << s[i].second << endl;
    else
        cout << "No news groups" << endl;
    done();
}

void createNG(Connection& conn){
    cout << "Create NewsGroup" << endl;
    string name;
    cout << "Title: ";
    cin >> name;

    if(createNGClient(conn, name))
        cout << "NewsGroup created" << endl;
    else
        cout << "NewsGroup already exists" << endl;
    done();
}

void deleteNG(Connection& conn){
    int nbr;
    cout << "NewsGroup id to delete: ";
    cin >> nbr;

    if(deleteNGClient(conn, nbr))
        cout << "NewsGroup deleted" << endl;
    else
        cout << "NewsGroup does not exists" << endl;
    done();
}

void listArt(Connection& conn){
    int nbr;
    cout << "NewsGroup id to show articles: ";
    cin >> nbr;

    vector<pair<int,string> > s = listArtClient(conn, nbr);

    if(s.size() > 0){
        if(s[0].first == -1)
            cout << s[0].second << endl;
        else {
            cout << "NewsGroup articles" << endl;
            for (size_t i = 0; i < s.size(); ++i)
                cout << s[i].first << " " << s[i].second << endl;
        }
    } else
        cout << "No NewsGroup articles" << endl;
    done();
}

void createArt(Connection& conn){
    int nbr;
    string sub, aut, text, tmp;

    cout << "NewsGroup id to add article: ";
    cin >> nbr;

    cout << "Enter subject: ";
    cin.ignore();
    getline(cin, sub);

    cout << "Enter author: ";
    getline(cin, aut);

    cout << "Enter text (save with 'save;' on a new line): ";
    while(!cin.eof()){
        getline(cin, tmp);
        if(tmp.compare("save;") == 0)
            break;
        text += tmp + '\n';
    }

    if(createArtClient(conn, nbr, sub, aut, text))
        cout << "Article created" << endl;
    else
        cout << "NewsGroup does not exists" << endl;
    done();
}

void deleteArt(Connection& conn){
    int nbrNg, nbrArt;
    cout << "Select NewsGroup the article exists in: ";
    cin >> nbrNg;

    cout << "Enter the id of the article to delete: ";
    cin >> nbrArt;

    int i = deleteArtClient(conn, nbrNg, nbrArt);

    if(i == -1)
        cout << "NewsGroup does not exists" << endl;
    else if(i == -2)
        cout << "Article does not exists" << endl;
    else
        cout << "Article deleted" << endl;
    done();
}

void readArt(Connection& conn){
    int nbrNg, nbrArt;
    cout << "Select NewsGroup the article exists in: ";
    cin >> nbrNg;

    cout << "Enter the id of the article to read: ";
    cin >> nbrArt;

    vector<string> s = readArtClient(conn, nbrNg, nbrArt);

    if(s.size() > 1)
        cout << s[0] << s[1] << s[2];
    else
        cout << s[0];
    done();
}

void menu(){
    system("clear");
    cout<<"   __  __                     __     _   __                 \n"
    <<"  / / / /_______  ____  ___  / /_   / | / /__ _      _______\n"
    <<" / / / / ___/ _ \\/ __ \\/ _ \\/ __/  /  |/ / _ \\ | /| / / ___/\n"
    <<"/ /_/ (__  )  __/ / / /  __/ /_   / /|  /  __/ |/ |/ (__  ) \n"
    <<"\\____/____/\\___/_/ /_/\\___/\\__/  /_/ |_/\\___/|__/|__/____/  \n";

    cout << "\n Usenet news publication\n";
    cout << " -----------------------------------------\n";
    cout << "|  Enter 1 to list news groups            |\n";
    cout << "|  Enter 2 to create news group           |\n";
    cout << "|  Enter 3 to delete news group           |\n";
    cout << "|  Enter 4 to list articles in news group |\n";
    cout << "|  Enter 5 to create article              |\n";
    cout << "|  Enter 6 to delete article              |\n";
    cout << "|  Enter 7 to read article                |\n";
    cout << "|  Enter 0 to clear and show menu         |\n";
    cout << "|  Enter Q to exit                        |\n";
    cout << " -----------------------------------------\n";
    cout << "Enter choice: ";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: myclient host-name port-number" << endl;
        exit(1);
    }

    Connection conn(argv[1], atoi(argv[2]));
    if (! conn.isConnected()) {
        cerr << "Connection attempt failed" << endl;
        exit(1);
    }

    menu();
    int nbr;
    while (cin >> nbr) {
        try {
            switch(nbr){
                case 0: menu(); break;
                case Protocol::COM_LIST_NG: listNG(conn); break;
                case Protocol::COM_CREATE_NG: createNG(conn); break;
                case Protocol::COM_DELETE_NG: deleteNG(conn); break;
                case Protocol::COM_LIST_ART: listArt(conn); break;
                case Protocol::COM_CREATE_ART: createArt(conn); break;
                case Protocol::COM_DELETE_ART: deleteArt(conn); break;
                case Protocol::COM_GET_ART: readArt(conn); break;
                default: cout << "Non valid selection\nEnter choice: ";
            }
        }
        catch (ConnectionClosedException&) {
            cerr << "Server closed down!" << endl;
            exit(1);
        }
    }
}