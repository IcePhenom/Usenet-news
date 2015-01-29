#include "connection.h"
#include "connectionclosedexception.h"
#include "protocol.h"
#include "message.h"

#include <vector>
#include <string>

using namespace std;
using namespace client_server;
using protocol::Protocol;

namespace message{

	void writeNumber(int value, Connection& conn) {
	    conn.write((value >> 24) & 0xFF);
	    conn.write((value >> 16) & 0xFF);
	    conn.write((value >> 8)  & 0xFF);
	    conn.write(value & 0xFF);
	}

	int readNumber(Connection& conn) {
	    unsigned char byte1 = conn.read();
	    unsigned char byte2 = conn.read();
	    unsigned char byte3 = conn.read();
	    unsigned char byte4 = conn.read();
	    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
	}

	void writeString(const string& s, Connection& conn) {
	    for (size_t i = 0; i < s.size(); ++i)
	        conn.write(s[i]);
	}

	string readString(int N, Connection& conn) {
	    string s;
	    for(int i = 0; i < N; ++i)
	        s += conn.read();
	    return s;
	}

	void writeByte(int value, Connection& conn){
	    conn.write(value);
	}

	int readByte(Connection& conn){
	    return conn.read();
	}

	vector<pair<int,string> > listNGClient(Connection& conn){
		vector<pair<int,string> > ret;
		writeByte(Protocol::COM_LIST_NG, conn);
	    writeByte(Protocol::COM_END, conn);
	    if(readByte(conn) == Protocol::ANS_LIST_NG){
	        if(readByte(conn) == Protocol::PAR_NUM){
	            int nbrNG = readNumber(conn);
	            for(int i = 0; i < nbrNG; ++i){
	                if(readByte(conn) == Protocol::PAR_NUM){
	                    int id = readNumber(conn);
	                    if(readByte(conn) == Protocol::PAR_STRING){
	                        int length = readNumber(conn);
	                        string title = readString(length, conn);
	                        ret.push_back(make_pair(id, title));
	                    }
	                }
	            }
	            if(readByte(conn) == Protocol::ANS_END){ }
   	        }
	    }
	    return ret;
	}

	bool createNGClient(Connection& conn, string name){
	    writeByte(Protocol::COM_CREATE_NG, conn);
	    writeByte(Protocol::PAR_STRING, conn);
	    writeNumber(name.size(), conn);
	    writeString(name, conn);
	    writeByte(Protocol::COM_END, conn);
	    if(readByte(conn) == Protocol::ANS_CREATE_NG){
	        if(readByte(conn) == Protocol::ANS_ACK){
	            if(readByte(conn) == Protocol::ANS_END)
	            	return true;
	        } else if(readByte(conn) == Protocol::ERR_NG_ALREADY_EXISTS){
	            if(readByte(conn) == Protocol::ANS_END)
	            	return false;
	        }
	    }
	    return false;
	}

	bool deleteNGClient(Connection& conn, int nbr){
	    writeByte(Protocol::COM_DELETE_NG, conn);
	    writeByte(Protocol::PAR_NUM, conn);
	    writeNumber(nbr, conn);
	    writeByte(Protocol::COM_END, conn);
	    if(readByte(conn) == Protocol::ANS_DELETE_NG){
	        if(readByte(conn) == Protocol::ANS_ACK){
	            if(readByte(conn) == Protocol::ANS_END)
	            	return true;
	        } else if(readByte(conn) == Protocol::ERR_NG_DOES_NOT_EXIST){
	            if(readByte(conn) == Protocol::ANS_END)
	                return false;
	        }
	    }
	    return false;
	}

	vector<pair<int,string> > listArtClient(Connection& conn, int nbr){
	    vector<pair<int,string> > ret;
	    writeByte(Protocol::COM_LIST_ART, conn);
	    writeByte(Protocol::PAR_NUM, conn);
	    writeNumber(nbr, conn);
	    writeByte(Protocol::COM_END, conn);
	    if(readByte(conn) == Protocol::ANS_LIST_ART){
	        if(readByte(conn) == Protocol::ANS_ACK){
	            if(readByte(conn) == Protocol::PAR_NUM){
	                int nbrArt = readNumber(conn);
	                for (int i = 0; i < nbrArt; ++i){
	                    if(readByte(conn) == Protocol::PAR_NUM){
	                    	int id = readNumber(conn);
	                        if(readByte(conn) == Protocol::PAR_STRING){
	                            int length = readNumber(conn);
	                            string title = readString(length, conn);
	                            ret.push_back(make_pair(id, title));
	                        }

	                    }
	                }
	            }
	            if(readByte(conn) == Protocol::ANS_END){ }
	        } else if(readByte(conn) == Protocol::ERR_NG_DOES_NOT_EXIST){
	        	ret.push_back(make_pair(-1, "NewsGroup does not exists"));
	            if(readByte(conn) == Protocol::ANS_END){ }
	        }
	    }
	    return ret;
	}

	bool createArtClient(Connection& conn, int nbr, string sub, string aut, string text){
	    writeByte(Protocol::COM_CREATE_ART, conn);
	    writeByte(Protocol::PAR_NUM, conn);

	    writeNumber(nbr, conn);
	    writeByte(Protocol::PAR_STRING, conn);

	    writeNumber(sub.size(), conn);
	    writeString(sub, conn);
	    writeByte(Protocol::PAR_STRING, conn);

	    writeNumber(aut.size(), conn);
	    writeString(aut, conn);
	    writeByte(Protocol::PAR_STRING, conn);

	    writeNumber(text.size(), conn);
	    writeString(text, conn);
	    writeByte(Protocol::COM_END, conn);
	    if(readByte(conn) == Protocol::ANS_CREATE_ART){
	        if(readByte(conn) == Protocol::ANS_ACK){
	            if(readByte(conn) == Protocol::ANS_END)
	            	return true;
	        } else if(readByte(conn) == Protocol::ERR_NG_DOES_NOT_EXIST){
	            if(readByte(conn) == Protocol::ANS_END)
	                return false;
	        }
	    }
	    return false;
	}

	int deleteArtClient(Connection& conn, int nbrNg, int nbrArt){
	    writeByte(Protocol::COM_DELETE_ART, conn);

	    writeByte(Protocol::PAR_NUM, conn);
	    writeNumber(nbrNg, conn);

	    writeByte(Protocol::PAR_NUM, conn);
	    writeNumber(nbrArt, conn);
	    writeByte(Protocol::COM_END, conn);
	    if(readByte(conn) == Protocol::ANS_DELETE_ART){
	        if(readByte(conn) == Protocol::ANS_ACK){
	            if(readByte(conn) == Protocol::ANS_END)
	            	return 0;
	        } else {
	            if(readByte(conn) == Protocol::ERR_NG_DOES_NOT_EXIST){
	                if(readByte(conn) == Protocol::ANS_END)
	                	return -1;
	            } else {
	                if(readByte(conn) == Protocol::ANS_END)
	                	return -2;
	            }
	        }
	    }
	    return -1;
	}

	vector<string> readArtClient(Connection& conn, int nbrNg, int nbrArt){
		vector<string> ret;
	    writeByte(Protocol::COM_GET_ART, conn);

	    writeByte(Protocol::PAR_NUM, conn);
	    writeNumber(nbrNg, conn);

	    writeByte(Protocol::PAR_NUM, conn);
	    writeNumber(nbrArt, conn);
	    writeByte(Protocol::COM_END, conn);
	    if(readByte(conn) == Protocol::ANS_GET_ART){
	        if(readByte(conn) == Protocol::ANS_ACK){
	            if(readByte(conn) == Protocol::PAR_STRING)
	            	ret.push_back("Title: " + readString(readNumber(conn), conn) + "\n");
	            if(readByte(conn) == Protocol::PAR_STRING)
	            	ret.push_back("Author: " + readString(readNumber(conn), conn) + "\n");
	            if(readByte(conn) == Protocol::PAR_STRING)
	            	ret.push_back("\n" + readString(readNumber(conn), conn));
	            if(readByte(conn) == Protocol::ANS_END){ }
	        } else {
	            if(readByte(conn) == Protocol::ERR_NG_DOES_NOT_EXIST){
	                if(readByte(conn) == Protocol::ANS_END)
	                	ret.push_back("NewsGroup does not exists");
	            } else {
	                if(readByte(conn) == Protocol::ANS_END)
	                	ret.push_back("Article does not exists");
	            }
	        }
	    }
	    return ret;
	}
}