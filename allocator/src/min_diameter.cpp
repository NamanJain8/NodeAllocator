#include <bits/stdc++.h>
#include <sqlite3.h>

using namespace std;

int NUM_HOSTS;
#define DEBUG 1

map<string, vector<long double> > allData;
vector< vector<long double> > bandwidth;
vector< vector<long double> > latency;
vector<string> hostnames;

string homedir = "/users/btech/akashish";

static int callback_node(void *NotUsed, int argc, char **argv, char **azColName) {
	for(int i = 0; i<argc; i++) {
		string field(azColName[i]);
		if(field=="node"){
			string s(argv[i]);
			allData[field].push_back( stoi(s.substr(5,s.length())) );
			continue;
		}
		allData[field].push_back(stod(argv[i]));
	}
	return 0;
}

static int callback_bw(void *NotUsed, int argc, char **argv, char **azColName) {
	int u,v;
	long double bw;
	for(int i = 0; i<argc; i++) {
		string field(azColName[i]);
		if(field=="hostA")
			u = stoi(argv[i]);
		else if (field=="hostB")
			v = stoi(argv[i]);
		else if (field=="bw")
			bw = stod(argv[i]);
	}

	int uidx = find(allData["node"].begin(), allData["node"].end(), u) - allData["node"].begin();
	int vidx = find(allData["node"].begin(), allData["node"].end(), v) - allData["node"].begin();
	if (uidx!=NUM_HOSTS and vidx!=NUM_HOSTS){
		bandwidth[uidx][vidx] = bw;
		bandwidth[vidx][uidx] = bw;
	}

	if(uidx==vidx)
		bandwidth[uidx][vidx] = 0;
	return 0;
}


static int callback_latency(void *NotUsed, int argc, char **argv, char **azColName) {
	int u,v;
	long double bw;
	for(int i = 0; i<argc; i++) {
		string field(azColName[i]);
		if(field=="hostA")
			u = stoi(argv[i]);
		else if (field=="hostB")
			v = stoi(argv[i]);
		else if (field=="latency")
			bw = stod(argv[i]);
	}

	int uidx = find(allData["node"].begin(), allData["node"].end(), u) - allData["node"].begin();
	int vidx = find(allData["node"].begin(), allData["node"].end(), v) - allData["node"].begin();
	if (uidx!=NUM_HOSTS and vidx!=NUM_HOSTS){
		latency[uidx][vidx] = bw;
		latency[vidx][uidx] = bw;
	}

	if(uidx==vidx)
		latency[uidx][vidx] = 0;
	return 0;
}

sqlite3* open_db(string hostname, FILE* logfile){
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	string datadir = homedir + "/.eagle/" + hostname + "/data.db";
	rc = sqlite3_open(datadir.c_str(), &db);
	if( rc ) {
		fprintf(logfile, "Can't open database for %s: %s %d\n",hostname.c_str(), sqlite3_errmsg(db), rc);
		return NULL;
	} else {
		fprintf(logfile, "Opened database successfully %s\n", hostname.c_str());
	}
	return db;
}

int exec_db(sqlite3* db, string sql, int (*callback)(void *NotUsed, int argc, char **argv, char **azColName), string hostname, FILE* logfile){
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
	if(rc == SQLITE_CORRUPT or rc==SQLITE_ERROR){
		return rc;
	}
	while(rc!=SQLITE_OK){
		cout << "loop ";
		rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
	}
	if( rc != SQLITE_OK ){
		fprintf(logfile, "SQL error for %s: %s %d\n",hostname.c_str(), zErrMsg, rc);
		
		sqlite3_free(zErrMsg);
	} else {
		fprintf(logfile, "Records read successfully for %s\n", hostname.c_str());
	}
}

int read_db(string hostname, FILE* logfile){
	sqlite3* db = open_db(hostname, logfile);
	if(db==NULL)
		return SQLITE_CANTOPEN;
	char *zErrMsg = 0;
	int rc;

	// Get node data
	string sql = "SELECT * from node";
	int err = exec_db(db, sql, &callback_node, hostname, logfile);
	sqlite3_close(db);
	cout << "done";
	return err;
}


int read_db_bw(string hostname, FILE* logfile){
	
	sqlite3* db = open_db(hostname, logfile);
	if(db == NULL)
		return SQLITE_CANTOPEN;
	char *zErrMsg = 0;
	int rc;
	
	// Get bandwidth data
	string sql = "SELECT * from bw";
	int err = exec_db(db, sql, &callback_bw, hostname, logfile);
	sqlite3_close(db);
	cout << "done";
	return 0;
}

int read_db_latency(string hostname, FILE* logfile){
	
	sqlite3* db = open_db(hostname, logfile);
	if(db == NULL)
		return SQLITE_CANTOPEN;
	char *zErrMsg = 0;
	int rc;
	
	// Get bandwidth data
	string sql = "SELECT * from latency";
	int err = exec_db(db, sql, &callback_latency, hostname, logfile);
	sqlite3_close(db);
	return 0;
}


int read_nodedata(string hostname, FILE* logfile){
	string datafile = homedir + "/.eagle/" + hostname + "/eagle.txt";
  	ifstream myfile (datafile);
	if(myfile.is_open())
	{
		string line;
		getline (myfile,line);
		if(line=="") return -1;
		vector<string> token;
		istringstream iss(line);
		for(string s; iss >> s; )
			token.push_back(s);
		if(token.size() == 20){
			string field[20] = {"node", "cpucount" , "corecount", "cpufreqmin" , 
				"cpufreqcurrent" , "cpufreqmax" , "load_1", "load_5", "load_15","band_10", "band_50", "band_150", "util_10", 
				"util_50", "util_150", "memory", "memory_10", "memory_50", "memory_150",  "nodeusers"};
							
			allData["node"].push_back( stoi(token[0].substr(5,token[0].length())));
			
			for(int i=1;i<20;i++){
				allData[field[i]].push_back( stold(token[i]) );
			}
		}else
		{
			return -1;
		}
		
	} else{
		cout << "Unable to open node data file for " << hostname << endl;
		return -1; 
	}
	return 0;
}

void read_latency(string hostname, FILE* logfile){
	string datafile = homedir + "/.eagle/" + hostname + "/latency.txt";
  	ifstream myfile (datafile);
	if(myfile.is_open())
	{
		string line;
		while ( getline (myfile,line) )
		{
			vector<string> token;
			istringstream iss(line);
			for(string s; iss >> s; )
    			token.push_back(s);
			if(token.size() == 3){
				int u = stoi(token[0].substr( 5,token[0].length() ) );
				int v = stoi(token[1].substr( 5,token[1].length() ) );
				int l = stold(token[2]);

				int uidx = find(allData["node"].begin(), allData["node"].end(), u) - allData["node"].begin();
				int vidx = find(allData["node"].begin(), allData["node"].end(), v) - allData["node"].begin();
				if (uidx!=NUM_HOSTS and vidx!=NUM_HOSTS){
					latency[uidx][vidx] = l;
					latency[vidx][uidx] = l;
				}
				if(uidx==vidx)
					latency[uidx][vidx] = 0;
			}
		}
	} else{
		cout << "Unable to open latency file for " << hostname << endl; 
	}
}

void read_bw(string hostname, FILE* logfile){
	string datafile = homedir + "/.eagle/" + hostname + "/bw.txt";
  	ifstream myfile (datafile);
	int n = 0;
	if(myfile.is_open())
	{
		
		string line;
		while ( getline (myfile,line) )
		{

			vector<string> token;
			istringstream iss(line);
			for(string s; iss >> s; )
    			token.push_back(s);
			if(token.size() == 3){
				int u = stoi(token[0].substr( 5,token[0].length() ) );
				int v = stoi(token[1].substr( 5,token[1].length() ) );
				int l = stold(token[2]);

				int uidx = find(allData["node"].begin(), allData["node"].end(), u) - allData["node"].begin();
				int vidx = find(allData["node"].begin(), allData["node"].end(), v) - allData["node"].begin();
				if (uidx!=NUM_HOSTS and vidx!=NUM_HOSTS){
					n = n + 1;
					bandwidth[uidx][vidx] = l;
					bandwidth[vidx][uidx] = l;
				}
				if(uidx==vidx)
					bandwidth[uidx][vidx] = 0;
				}
		}
	} else{
		cout << "Unable to open bandwidth file for " << hostname << endl; 
	}
	cout << n << hostname << " ";
}

void compute_process_count(){
	for(int i=0;i<NUM_HOSTS;i++){
		long double loadAvg = ( 5*allData["load_1"][i] + 3*allData["load_1"][i] + 2*allData["load_1"][i]) / 10.0;
		int processCount = allData["cpucount"][i] - (int)fmod(loadAvg,allData["cpucount"][i]);
		allData["processcount"].push_back(processCount);

		allData["load_1"][i] = allData["load_1"][i] / allData["cpucount"][i];
		allData["load_5"][i] = allData["load_5"][i] / allData["cpucount"][i];
		allData["load_15"][i] = allData["load_15"][i] / allData["cpucount"][i];

	}

	return;
}

void compute_power(long double comp_power[], map<string, long double> weights){
	for(int i=0;i<NUM_HOSTS;i++){
		map<string, long double>::iterator it;
		for(it=weights.begin();it!=weights.end();it++){
			string key = it->first;
			if(weights[key]<0){
				comp_power[i] += (-weights[key])*(1-allData[key][i]);
			}
			else	
				comp_power[i] += weights[key]*allData[key][i];
		}
	}
	if(DEBUG)
	{
		cout << "COMPUTE POWER OF EACH NODE W/o NORM" << endl;
		for(int idx=0;idx<NUM_HOSTS;idx++)
			cout<<comp_power[idx]<<" ";
		cout<<endl;
	}
	transform(comp_power,comp_power+NUM_HOSTS , comp_power, bind2nd(divides<long double>(), accumulate(comp_power, comp_power+NUM_HOSTS, 0)));
	
	if(DEBUG)
	{
		cout << "COMPUTE POWER OF EACH NODE" << endl;
		for(int idx=0;idx<NUM_HOSTS;idx++)
			cout<<comp_power[idx]<<" ";
		cout<<endl;
	}
}

void set_weights(map<string, long double> &weights){

	weights.insert({"cpucount", 2});
	weights.insert({"cpufreqmax", 2});
	weights.insert({"corecount", 2});

	weights.insert({"load_1", -2});
	weights.insert({"load_5", -2});
	weights.insert({"load_15", -2});

	weights.insert({"band_10", -8});
	weights.insert({"band_50", -7});
	weights.insert({"band_150", -6});

	weights.insert({"util_10", -2});
	weights.insert({"util_50", -2});

	weights.insert({"memory", 2});
	weights.insert({"memory_50", 2});
	weights.insert({"nodeusers",-7});
}

// int Diameter(vector<int> V, vector<vector<long double> > l){
// 	int v_size = V.size();
// 	int diameter = 0;
// 	for(int i=0;i<v_size;i++){
// 		for(int j=i+1;j<v_size;j++){
// 			int u,v;
// 			u = V[i];
// 			v = V[j];
// 			diameter = max(l[u][v], long double(diameter));
// 		}
// 	}
// 	return diameter;
// }

vector<int> FindMinStar(vector<int> G, vector<int> counts, int node, int n){
	
	vector<int> V_;
	V_.push_back(node);

	int i = 1;
	int size = counts[node];

	vector<long double > bw_array;
	vector<long double > latency_array;

	vector<long double>::iterator it;
	for(it = bandwidth[node].begin(); it != bandwidth[node].end();it++){
		int u = *it;
		bw_array.push_back({bandwidth[node][u]});
		latency_array.push_back({latency[node][u]});
	}

	long double band_sum =  accumulate(bw_array.begin(), bw_array.end(), 0.0);
	if(band_sum==0) band_sum = 1;
	transform(bw_array.begin(), bw_array.end(), bw_array.begin() , bind2nd(divides<long double>(), band_sum));

	long double l_sum =  accumulate(latency_array.begin(), latency_array.end(), 0.0);
	if(l_sum==0) l_sum = 1;
	for(int i=0; i<latency_array.size(); ++i){
		latency_array[i] = l_sum - latency_array[i];
	}
	l_sum =  accumulate(latency_array.begin(), latency_array.end(), 0.0);
	transform(latency_array.begin(), latency_array.end(), latency_array.begin() , bind2nd(divides<long double>(), l_sum));

	vector<pair<double,int> > netw_array;
	for(int i=0; i< bw_array.size();i++){
		long double netw = 0.9*bw_array[i] + 0.1*latency_array[i];
		netw_array.push_back({netw, i});
	}
	sort(netw_array.begin(),netw_array.end(), greater<pair<long double, int> >());

	while(size<n and i<netw_array.size()){
		int ui = netw_array[i].second;
		i = i + 1;
		if(find(G.begin(), G.end(), ui) == G.end() or ui==node)
			continue;
		V_.push_back(ui);
		size = size + counts[ui];
	}

	if(size<n){
		vector<int> tmp;
		return tmp;
	}
	return V_;
}

long double compute_Comp(vector<int> G, long double comp_power[]){
	long double power = 0;
	for(int i=0;i<G.size();i++){
		power += comp_power[G[i]];
	}
	return power;
}

long double compute_Netw(vector<int> G){
	long double power = 0;
	for(int i=0;i<G.size();i++){
		for(int j=0;j<G.size();j++){
			power += bandwidth[G[i]][G[j]];
		}
	}
	if(G.size()==1)
		return 10000;
	power /= (G.size()*G.size());
	return power;
}

vector<int> FindBestStar(vector<int> G, vector<int> counts, int n, long double comp_power[]){
	
	vector<int> G_;
	long double comp[NUM_HOSTS] = {0};
	long double netw[NUM_HOSTS] = {0};
	long double latency[NUM_HOSTS] = {1000000};
	long double total_power[NUM_HOSTS] = {0};

	// Calculate compute and network power
	for(auto it=G.begin();it!=G.end();it++){
		int node = int(*it);
		vector<int> G__ = FindMinStar(G, counts, node, n);
		if (G__.size()==0){
			continue;
		}
		comp[node] = compute_Comp(G__, comp_power);
		netw[node] = compute_Netw(G__);
	}

	// Normalize it
	long double comp_sum = accumulate(comp, comp+NUM_HOSTS, 0.0);
	if(comp_sum==0) comp_sum = 1;
	transform(comp, comp + NUM_HOSTS, comp, bind2nd(divides<long double>(),comp_sum ));

	long double band_sum =  accumulate(netw, netw+NUM_HOSTS, 0.0);
	if(band_sum==0) band_sum = 1;
	transform(netw, netw + NUM_HOSTS, netw, bind2nd(divides<long double>(), band_sum));

	// Compute total power
	int max_idx = 0;
	long double max_pow = -1;
	for(int i=0;i<NUM_HOSTS;i++){
		total_power[i] = 0.1*comp[i] + 0.9*netw[i];
		if(total_power[i] > max_pow){
			max_pow = total_power[i];
			max_idx = i;
		}
	}

	if(DEBUG==1){
		cout<<"Computation, Network and Total power for each node star:\n"<<endl;
		for(int i=0;i<NUM_HOSTS;i++){
			cout<<allData["node"][i]<<" "<<comp[i]<<" "<<netw[i]<<" "<<total_power[i]<<endl;
		}
		cout<<"Max:"<<max_pow<<" "<<allData["node"][max_idx]<<endl;
	}

	G_ = FindMinStar(G, counts, max_idx, n);
	return G_;
}

void getLiveHosts(string filename){
	ifstream hostfile(filename);
	string hostname;
	while(hostfile >> hostname){
		cout<<hostname<<" ";
		hostnames.push_back(hostname);
	}
	NUM_HOSTS = hostnames.size();
	cout<<endl<<"Number of hosts: "<<NUM_HOSTS<<endl;
}

int main(int argc, char **argv){
	if(argc<3){
		cout<<"Usage: <executable> <number of hosts> <ppn>"<<endl;
		exit(0);
	}

	int required_cores = atoi(argv[1]);
	int ppn  = atoi(argv[2]);

	// Get list of hosts
	getLiveHosts(homedir+"/.eagle/livehosts.txt");
	
	FILE *logfile;
	logfile = fopen("allocator.log","w");

	// Initialize bandwidth
	for(int i=0;i<NUM_HOSTS;i++){
		vector<long double> tmp;
		for(int j=0;j<NUM_HOSTS;j++){
			tmp.push_back(0);
		}
		bandwidth.push_back(tmp);
	}

	// Initialize latency
	for(int i=0;i<NUM_HOSTS;i++){
		vector<long double> tmp;
		for(int j=0;j<NUM_HOSTS;j++){
			tmp.push_back(3000);
		}
		latency.push_back(tmp);
	}

	auto start = time(NULL); 
	
	// // Read node data
	// for(int idx=0; idx<NUM_HOSTS; idx++){
	// 	int err = read_db(hostnames[idx], logfile);
	// 	if(err == SQLITE_CORRUPT or err==SQLITE_ERROR or err==SQLITE_CANTOPEN){
	// 		hostnames.erase(idx + hostnames.begin());
	// 		idx--;
	// 		NUM_HOSTS--;
	// 	}
	// }

	// // Read bandwidth data
	// for(int idx=0; idx<NUM_HOSTS; idx++){
	// 	int err1 = read_db_bw(hostnames[idx], logfile);
	// 	// int err2 = read_db_latency(hostnames[idx], logfile);
	// 	int flag1 = (err1 == SQLITE_CORRUPT or err1==SQLITE_ERROR or err1==SQLITE_CANTOPEN);
	// 	// int flag2 = (err2 == SQLITE_CORRUPT or err2==SQLITE_ERROR or err2==SQLITE_CANTOPEN);
	// 	if(flag1){
	// 		hostnames.erase(idx + hostnames.begin());
	// 		idx--;
	// 		NUM_HOSTS--;
	// 	}
	// }

	auto rstart = time(NULL); 
	for(int idx=0; idx<NUM_HOSTS; idx++){
		int rc = read_nodedata(hostnames[idx], logfile);
		if(rc==-1){
			hostnames.erase(idx + hostnames.begin());
			idx--;
			NUM_HOSTS--;
		}
	}
	for(int idx=0; idx<NUM_HOSTS; idx++){
		read_latency(hostnames[idx], logfile);
		read_bw(hostnames[idx], logfile);
	}

	auto rend = time(NULL); 
	auto rduration = (rend - rstart); 
	if(DEBUG)
		cout<<endl<<"Data read time: "<< rduration<<endl;


	if(DEBUG){
		cout<<"Bandwidth Data :\n";
		for(int i=0;i<NUM_HOSTS;i++){
			for(int j=0;j<NUM_HOSTS;j++)
				cout<<bandwidth[i][j]<<" ";
			cout<<endl;
		}
		cout<<"Latency Data :\n";
		for(int i=0;i<NUM_HOSTS;i++){
			for(int j=0;j<NUM_HOSTS;j++)
				cout<<latency[i][j]<<" ";
			cout<<endl;
		}
		// cout << "Node Data :\n";
		// for(int i=0;i<NUM_HOSTS;i++){
		// 	map<string, vector< long double> >::iterator it;
		// 	for(it=allData.begin();it!=allData.end();it++){
		// 		string key = it->first;
		// 		cout << key << ":" << allData[key][i] << " ";
		// 	}
		// 	cout << endl;
		// }
	}

	//Find Proccess count to be allocated on each node
	compute_process_count();
	if(DEBUG){
		cout << "Process and Cpu Count\n ";
		for(int i=0;i< NUM_HOSTS; i++){
			cout << allData["processcount"][i] << "," << allData["cpucount"][i] << " ";
		}
		cout << endl;
	}
	if(ppn!=0){
		for(int i=0;i<NUM_HOSTS;i++){
			allData["processcount"][i] = ppn;
		}
	}
	

	// Normalize node data
	map<string, vector<long double> >::iterator it;
	for(it=allData.begin();it!=allData.end();it++){
		string field = it->first;
		if(field=="node" or field=="processcount")
			continue;
		// Normalize
		transform(allData[field].begin(), allData[field].end(), allData[field].begin(), bind2nd(divides<long double>(), accumulate(allData[field].begin(), allData[field].end(), 0.0)));
	}

	
	vector<int> active_nodes;
	vector<int> core_count;

	for(int i=0;i<NUM_HOSTS;i++)
		core_count.push_back(int(allData["processcount"][i]));
	for(int i=0;i<NUM_HOSTS;i++)
		active_nodes.push_back(i);

	if(DEBUG){
		cout<<endl<<"Nodes"<<endl;
		for(int i=0;i<NUM_HOSTS;i++){
			cout<<allData["node"][i]<<" ";
		}
		cout<<endl;
		for(int i=0;i<NUM_HOSTS;i++){
			cout<<hostnames[i]<<" ";
		}
		cout<<endl;
	}

	map<string, long double> weights;
	set_weights(weights);
	long double comp_power[NUM_HOSTS] = {0};
	compute_power(comp_power, weights);

	vector<int> G = FindBestStar(active_nodes, core_count, required_cores, comp_power);

	if(G.size()==0)
		cout<<"Not found\n";
	
	FILE *hosts;
	hosts = fopen("hosts","w");
	if(DEBUG){
		for(auto it = G.begin(); it!=G.end();it++){
			cout<<allData["node"][*it]<<" "<<allData["processcount"][*it]<<endl;
			fprintf(hosts, "csews%d:%d\n", int(allData["node"][*it]), int(allData["processcount"][*it]));
		}
		cout<<endl;
	}

	fclose(hosts);
	fclose(logfile);
	return 0;
}
