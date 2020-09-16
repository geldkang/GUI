
#ifndef __FUZZER_H__
#define __FUZZER_H__

#include "gui.h"

#include "sender.h"
#include "iGenerator.h"
#include "scheduler.h"
#include "global.h"
#include "exceptions.h"
#include "packet.h"

namespace wifi
{

int _fprint_payld(char* payld, int size, std::string type, int chk, int pkt_no, int state);

class Fuzzer
{
public:
	Fuzzer(sender &s);
	~Fuzzer();
	//Fuzzer(const Fuzzer &f);
	
	int fuzz(gui_fuzz* gui_fuzz) throw (std::runtime_error);
	int replay() throw (std::runtime_error);
	
	int setSchler(scheduler *sch);
	scheduler* getSchler();
	
	sender &s;
	iGenerator igen;
	scheduler *schler;
	//char prev_payload[PAYLOAD_SIZE];
	//int prev_size;
	
	/* fuzzing options */
	int opt_sleep_ms;
	int opt_iteration;
	
private:
	int set_default_opt();
	int sleep_ms();
	int set_timespec();
	
	int print_start();
	int print_end();
	int print_payload(char* p, int s);
	
	struct timespec ts;
	packet prev_pkt;
	int prev_state;
	
	int select_start_state();
};

} // namespace wifi

#endif
