#include <unistd.h>
#include "view.hpp"
#include "commo.hpp"
#include "mpaxos/mpaxos.h"
#include <string>
#include "tools.hpp"
//#include <sstream>

static unsigned char data_[20] = "Hello MPaxos!\n";
static int exit_ = 0;

static void cb(mpaxos_req_t *req) {
    printf("%s", req->data);
    exit_ = 1;
}

int main (int argc, char **argv) {

	typedef struct {
		uint32_t a;
		uint32_t b;
	} struct_add;

	funid_t ADD = 1;
	if(argc < 2 ) {
		printf("Usage: %s nodename\n", argv[0]);
		return -1;
	}
	mpaxos_init();
	mpaxos_config_load("./config/localhost-3.yaml");
//	std::string nodename = "node" + int2string(i+1);
	std::string nodename = argv[1];
	mpaxos_set_me(nodename);

	mpaxos_set_cb_god(cb);

//	mpaxos_commo_start();
/* TEST OK this part
	struct_add sa;
    sa.a = 1;
    sa.b = 2;
	std::string hostname = "node2";
//	mpaxos_commo_sendto(hostname, ADD, (uint8_t *)&sa, sizeof(struct_add));
	mpaxos_commo_sendto_all( ADD, (uint8_t *)&sa, sizeof(struct_add));
*/	
	//draft test go on now
    mpaxos_req_t *req = new mpaxos_req_t;
 //   memset(&req, 1, sizeof(mpaxos_req_t));
	std::string haha = "Hello Loli!";
    req->data = (uint8_t *)haha.c_str();
    req->sz_data = haha.length();

	mpaxos_enlist(req);
   // mpaxos_commit_req(&req);
	mpaxos_retire(req);

//	while(1);
//	mpaxos_commo_stop();
}
