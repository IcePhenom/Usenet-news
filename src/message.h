#include "connection.h"
#include "connectionclosedexception.h"
#include "protocol.h"

#include <string>
#include <vector>

using namespace std;
using namespace client_server;
using protocol::Protocol;

namespace message{

	void writeNumber(int value, Connection& conn);

	int readNumber(Connection& conn);

	void writeString(const string& s, Connection& conn);

	string readString(int N, Connection& conn);

	void writeByte(int value, Connection& conn);

	int readByte(Connection& conn);

	vector<pair<int,string> > listNGClient(Connection& conn);

	bool createNGClient(Connection& conn, string title);

	bool deleteNGClient(Connection& conn, int nbr);

	vector<pair<int,string> > listArtClient(Connection& conn, int nbr);

	bool createArtClient(Connection& conn, int nbr, string sub, string aut, string text);

	int deleteArtClient(Connection& conn, int nbrNg, int nbrArt);

	vector<string> readArtClient(Connection& conn, int nbrNg, int nbrArt);

}