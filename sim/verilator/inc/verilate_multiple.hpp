#pragma once
#include <zmq.hpp>
#include <tuple>


//
// When you include this file, your tb.cpp must declare these functions
//

void Board0Begin(HiggsHelper<top_t>* const t);
void Board1Begin(HiggsHelper<top_t>* const t);
int Board0Loop(HiggsHelper<top_t>* const t, const uint32_t us);
int Board1Loop(HiggsHelper<top_t>* const t, const uint32_t us);
int Board0Final(HiggsHelper<top_t>* const t, const uint32_t us);
int Board1Final(HiggsHelper<top_t>* const t, const uint32_t us);


#define assert2(condition) if(!(condition)){return 1;}






// ZMQ addresses

std::string zmq_peer_address = "ipc:///tmp/peer_0_1_";

std::string worker_0_exit_address = "ipc:///tmp/parent_worker_exit_0_";
std::string worker_1_exit_address = "ipc:///tmp/parent_worker_exit_1_";









typedef std::function<void(void)> inner_cb_t;
typedef std::function<inner_cb_t(const uint32_t)> outer_cb_t;




typedef std::function<void(const char* const d, const size_t len)> got_msg_cb_t;

typedef std::function<inner_cb_t(const uint32_t, zmq::socket_t& s0, zmq::socket_t& s1, got_msg_cb_t c0, got_msg_cb_t c1)> outer_dual_cb_t;

typedef std::function<inner_cb_t(const uint32_t, zmq::socket_t& s0, got_msg_cb_t c0)> outer_single_cb_t;





outer_single_cb_t GetSingleChecker = [](const uint32_t as, zmq::socket_t& s0, got_msg_cb_t c0) {
    cout << "parser boot as " << as << "\n";

    int& runs = *(new int(0));

    zmq::pollitem_t* items = new zmq::pollitem_t[1];

    items[0] = { s0, 0, ZMQ_POLLIN, 0 };
    // items[1] = { s1, 0, ZMQ_POLLIN, 0 };

    const uint32_t _poll_interval = 0;//100;//300; // in ms // FIXME what about 10

    return [&, as, _poll_interval, items, c0](void) {

        zmq::poll(items, 1, _poll_interval);
        
        if (items[0].revents & ZMQ_POLLIN) {
            // cout << "s0 has call\n";
            zmq::message_t reply;
            s0.recv(&reply);
            c0((const char*)reply.data(),reply.size());
        }



        // if (items[1].revents & ZMQ_POLLIN) {
        //     // cout << "s1 has call\n";
        //     zmq::message_t reply;
        //     s1.recv(&reply);
        //     c1((const char*)reply.data(),reply.size());
        // }

        if( as != 0 ) {
            cout << "run " << runs << " as " << as <<  "\n";
        }
        runs++;
        return;
    };
};




outer_dual_cb_t GetDualChecker = [](const uint32_t as, zmq::socket_t& s0, zmq::socket_t& s1, got_msg_cb_t c0, got_msg_cb_t c1) {
    cout << "parser boot as " << as << "\n";

    int& runs = *(new int(0));

    zmq::pollitem_t* items = new zmq::pollitem_t[2];

    items[0] = { s0, 0, ZMQ_POLLIN, 0 };
    items[1] = { s1, 0, ZMQ_POLLIN, 0 };

    // zmq::pollitem_t items [2] = {
    //     { worker0, 0, ZMQ_POLLIN, 0 }
    //     ,{ worker1, 0, ZMQ_POLLIN, 0 }
    //  };

    const uint32_t _poll_interval = 100;//300; // in ms // FIXME what about 10

    return [&, as, _poll_interval, items, c0, c1](void) {

        zmq::poll(items, 2, _poll_interval);
        
        if (items[0].revents & ZMQ_POLLIN) {
            // cout << "s0 has call\n";
            zmq::message_t reply;
            s0.recv(&reply);
            c0((const char*)reply.data(),reply.size());
        }



        if (items[1].revents & ZMQ_POLLIN) {
            // cout << "s1 has call\n";
            zmq::message_t reply;
            s1.recv(&reply);
            c1((const char*)reply.data(),reply.size());
        }

        if( as != 0 ) {
            cout << "run " << runs << " as " << as <<  "\n";
        }
        runs++;
        return;
    };
};






///
/// This is the code for the 2 higgs verilator situation
/// This is extreemly complicated, however I've tried to make it as simple and modular as possible
/// This function gets called immediately after the fork.  The argument tell us which of the higgs
/// processes we are.
/// The goal is to bind valid-ready ports between the two forked processes.  We do this using zmq
/// I tried to come up with a way to simplify sending things over zmq.
/// There are two vectors of lambdas.  One on the encoding side, one on the decoding side
/// These vectors are different on each fork.
/// The encoding vector on fork id 0 must match the decoding vector on fork id 1
/// There is a zmq socket beteen the fork id 0 and fork id 1.
/// the direction of bind/connect is determined by the id.
/// Also, each fork sleeps a bit at the start, longer for higher numbers
/// This should allow us to predict which fork starts when
void forked_work(const uint32_t worker_id, const uint32_t runtime_us)
{
    // int worker_id = ++who;



    bool exitAfterFirst = false;

    // should encourage higher # forks to wakeup later
    usleep(500*(worker_id+1));

    zmq::context_t context(1);

    // zmq::socket_t parent (context, ZMQ_REQ);
    // parent.connect (std::string("ipc://xxbind") + std::to_string(worker_id));

    zmq::socket_t exit_to_parent(context, ZMQ_REQ);
    if( worker_id == 0 ) {
        exit_to_parent.connect(worker_0_exit_address);
    } else {
        exit_to_parent.connect(worker_1_exit_address);
    }




    zmq::socket_t peer  (context, (worker_id==0)?ZMQ_REQ:ZMQ_REP);

    // const std::string zmq_peer_address = "ipc:///tmp/peer_0_1";

    if( worker_id == 0 ) {
        peer.bind(zmq_peer_address);
    } else {
        peer.connect(zmq_peer_address);
    }

    // zmq::socket_t server(context, ZMQ_REP);





    const std::string us = std::string("worker ") + std::to_string(worker_id);

    auto cpid = getpid();
    cout << (us + " boot with pid ") << cpid << std::endl;

    unsigned messages = 0;
    unsigned runs = 0;

    // const uint8_t prefix_empty = 0;  // do not care (ie empty)
    // const uint8_t prefix_high = 1;
    // const uint8_t prefix_low = 2;



    // auto CheckParent = GetSingleChecker(0, parent,
    //     [&](const char* const d, const size_t len) {

    //         // cout << us << " has data " << d << "\n";

    //         std::cout << us << " got reply: [" << d << "]" << std::endl;
    //     });

    std::string vcd_filename;
    if(worker_id == 0) {
        vcd_filename = "wave_dump.vcd";
    } else {
        vcd_filename = "wave_dump1.vcd";
    }

    TB_START_FILENAME(vcd_filename);

    // shared pulls must come before HiggsHelper
    // i dont know why but HiggsHelper seemds to mess with rand()
    // on the forks
    uint32_t shared_pull0 = rand();


    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    t->SetupMultipleHiggs(worker_id);
    preReset(top);
    t->reset(40);
    postReset(top);

    t->monitor_dac_valid = false;


    if( worker_id == 0 ) {
        Board0Begin(t);
    } else if( worker_id == 1 ) {
        Board1Begin(t);
    }


    // after calling this, rand() will diverge on the forks
    // so make sure to do any shared pulls before this
    t->ReseedAfterFork(worker_id);

    uint32_t unique_pull0 = rand() & 0xffffffff;
    uint32_t unique_pull1 = rand();
    uint32_t unique_pull2 = rand();

    cout << t->prefix << "Injecting seed " << HEX32_STRING(unique_pull0) << " into riscv memory\n";
    t->InjectTestSeed(unique_pull0);

    cout << t->prefix << "sp0 " << shared_pull0 << "\n";
    cout << t->prefix << "up1 " << unique_pull1 << "\n";
    cout << t->prefix << "up2 " << unique_pull2 << "\n";

    // t->tickHigh();
    // t->tickLow();

    // uint32_t runtime = 5;



    // for( auto p : t->ins ) {
    //     std::string a;
    //     Port32In* b;
    //     std::tie(a,b) = p;

        // cout << "a: " << "\n";
    // }

    // return;

    // zmq::message_t reply(9);
    // memcpy ((void*) reply.data(), "WorldX_X", 9);
    // ((char*)(reply.data())) [5] = '0' + worker_id;



    // for(const auto w : t->GetEnabledFPGAS()) {
    //     cout << w << "\n";
    // }

    // t->InjectTestSeed(4);



    Port32Out* const cs01 = t->outs["cs01out"];
    Port32In* const cs31 = t->ins["cs31in"];

    t->ins["cs31in"]->drive_port = false;
    t->outs["cs01out"]->drive_port = false;

    





    std::vector< std::function<std::vector<uint8_t>(void)> > ser_list;

    if( worker_id == 0 ) {
        ser_list.push_back([&](void) {
            return cs01->Serialize();
        });
        ser_list.push_back([&](void) {
            return cs31->Serialize();
        });

    } else {
        ser_list.push_back([&](void) {
            return cs31->Serialize();
        });
        ser_list.push_back([&](void) {
            return cs01->Serialize();
        });
    }









    // remember, we call Deserialize on Port32Out with the data from Port32In->Serialize()
    size_t d_size_Port32Out = cs31->Serialize().size();
    size_t d_size_Port32In = cs01->Serialize().size();
    size_t d_progress = 0;



    ///
    /// Build a tuple of size_t size_t fn()
    /// the size_t are the start and length of data
    /// we will build a vector from start, lenth, and then pass that vector to fn
    /// in this way, the zmq transport can be abstracted away
    ///
    std::vector< std::tuple<size_t,size_t,std::function<void(const std::vector<uint8_t>&)>> > deser_list;


    if( worker_id == 0 ) {
        deser_list.push_back(std::make_tuple(d_progress,d_size_Port32Out,[&](const std::vector<uint8_t>& x) {
            cs01->Deserialize(x);
        }));
        d_progress+=d_size_Port32Out;

        deser_list.push_back(std::make_tuple(d_progress,d_size_Port32In,[&](const std::vector<uint8_t>& x) {
            cs31->Deserialize(x);
        }));
        d_progress+=d_size_Port32In;
    } else {

        deser_list.push_back(std::make_tuple(d_progress,d_size_Port32In,[&](const std::vector<uint8_t>& x) {
            cs31->Deserialize(x);
        }));
        d_progress+=d_size_Port32In;

        deser_list.push_back(std::make_tuple(d_progress,d_size_Port32Out,[&](const std::vector<uint8_t>& x) {
            cs01->Deserialize(x);
        }));
        d_progress+=d_size_Port32Out;

    }


    uint32_t i = 0;
    for(i = 0; i < (runtime_us*500); i++) {

        t->tickHigh();

        std::vector<uint8_t> highData;

        for(const auto fn : ser_list) {
            const std::vector<uint8_t> part = fn();
            VEC_APPEND(highData, part);
        }

        zmq::message_t packetHigh(highData.size());
        memcpy(((uint8_t*)packetHigh.data()), highData.data(), highData.size());


        zmq::message_t gotPacketHigh;


        // This is the order of the following transaction

        // 0 sends
        // 1 gets
        // 1 sends
        // 0 gets

        if( worker_id == 0 ) {
            peer.send(packetHigh);
            peer.recv(&gotPacketHigh);
        } else if( worker_id == 1 ) {
            peer.recv(&gotPacketHigh);
            peer.send(packetHigh);
        }

        // at this point, each peer has the other peers serialized data

        for(const auto packed : deser_list) {
            size_t start;
            size_t len;
            std::function<void(std::vector<uint8_t>)> fn;
            std::tie(start,len,fn) = packed;

            std::vector<uint8_t> gotLowData;
            gotLowData.assign(((const uint8_t*)gotPacketHigh.data()+start), ((const uint8_t*)gotPacketHigh.data()+start)+len);
            fn(gotLowData);
        }




        //////////////////////////////////////////////////////







        t->tickLow();

        std::vector<uint8_t> lowData;

        for(const auto fn : ser_list) {
            const std::vector<uint8_t> part = fn();
            VEC_APPEND(lowData, part);
        }


        zmq::message_t packetLow(lowData.size());
        memcpy(((uint8_t*)packetLow.data()), lowData.data(), lowData.size());

        zmq::message_t gotPacketLow;



        // This is the order of the following transaction

        // 0 sends
        // 1 gets
        // 1 sends (empty)       (empty for now untill we do bi-directional valid ready)
        // 0 gets (empty)

        if( worker_id == 0 ) {
            peer.send(packetLow);
            peer.recv(&gotPacketLow);
        } else if( worker_id == 1 ) {
            peer.recv(&gotPacketLow);
            peer.send(packetLow);
        }

        // at this point, each peer has the other peers serialized data



        //
        for(const auto packed : deser_list) {
            size_t start;
            size_t len;
            std::function<void(std::vector<uint8_t>)> fn;
            std::tie(start,len,fn) = packed;


            std::vector<uint8_t> gotLowData;
            gotLowData.assign(((const uint8_t*)gotPacketLow.data()+start), ((const uint8_t*)gotPacketLow.data()+start)+len);
            fn(gotLowData);
        }








        if( (i % 500) == 0 ) {
            if( worker_id == 0 ) {
                Board0Loop(t, i / 500);
            } else if( worker_id == 1 ) {
                Board1Loop(t, i / 500);
            }
        }

        


        // cout << "after: " << HEX32_STRING(*cs31->t_data) << "\n";
        // cout << "asSer len " << asSer.size() << "\n";
        // cout << (int)asSer[0] << ", " << (int)asSer[1] << ", " << (int)asSer[2] << ", " << (int)asSer[3] << "\n";
        // cout << "H: " << HEX32_STRING(*cs01->i_data) << " v: " << (int)(*cs01->i_valid) <<  " r: " << (int)(*cs01->i_ready) << "\n";

        // cout << "L: " << HEX32_STRING(*cs01->i_data) << " v: " << (int)(*cs01->i_valid) <<  " r: " << (int)(*cs01->i_ready) << "\n";
        // cs31->deserLow(asSer);

    }

    int final_exit_code = 0;

    // should allow output to always be 0 first
    usleep(10000*(worker_id));

    if( worker_id == 0 ) {
        final_exit_code = Board0Final(t, i / 500);
    } else if( worker_id == 1 ) {
        final_exit_code = Board1Final(t, i / 500);
    }


    // this is done for both 0 and 1, no if needed

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;



    // Exit Code
    const int8_t exit_code = final_exit_code;
    zmq::message_t exitPacket(1);
    ((int8_t*)exitPacket.data())[0] = exit_code;
    exit_to_parent.send(exitPacket);



    // cout << "\n";
    // cout << "\n";
    // cout << "\n";

    // for(const auto w : cs01->data) {
    //     cout << HEX32_STRING(w) << "\n";
    // }


        // for( auto p : t->outs ) {
        //     std::string a;
        //     Port32Out* b;
        //     std::tie(a,b) = p;

        //     cout << "a: " << a << "\n";
        // }

}




int VerilateMultiple(
    const int argc,
    char** const argv,
    char** const env,
    const uint32_t runtime_us) {

    // return TestSerDeser(argc, argv, env);

    auto parent_pid_s = std::to_string(getpid());
    // cout << "parent " << parent_pid_s << "\n";

    // use a unique socket name per run
    // this will prevent two runs of any forked verilator from clashing
    zmq_peer_address      += parent_pid_s;
    worker_0_exit_address += parent_pid_s;
    worker_1_exit_address += parent_pid_s;

    // return 0;

    // who = -1; // first will be 0

    //  Prepare our context and sockets
    zmq::context_t context (1);
    // zmq::socket_t clients (context, ZMQ_ROUTER);
    // clients.bind ("tcp://*:5555");
    // zmq::socket_t worker0 (context, ZMQ_REP);
    // worker0.bind ("ipc://xxbind0");
    // zmq::socket_t worker1 (context, ZMQ_REP);
    // worker1.bind ("ipc://xxbind1");



    zmq::socket_t worker0_exit(context, ZMQ_REP);
    worker0_exit.bind(worker_0_exit_address);
    zmq::socket_t worker1_exit(context, ZMQ_REP);
    worker1_exit.bind(worker_1_exit_address);



    TB_START_PRE();


    //  Launch pool of worker threads
    for (uint32_t fork_number = 0; fork_number != 2; fork_number++) {
        if(fork()==0) {
            forked_work(fork_number, runtime_us);
            exit(0);
        }
        // pthread_t worker;
        // pthread_create (&worker, NULL, worker_routine, (void *) &context);
    }



    zmq::message_t zmq_exit_0;
    worker0_exit.recv(&zmq_exit_0);

    zmq::message_t zmq_exit_1;
    worker1_exit.recv(&zmq_exit_1);


    const int exit_code_0 = ((int8_t*)zmq_exit_0.data())[0];
    const int exit_code_1 = ((int8_t*)zmq_exit_1.data())[0];

    cout << "Exit code from Higgs 0: " << exit_code_0 << "\n";
    cout << "Exit code from Higgs 1: " << exit_code_1 << "\n";

    if( exit_code_0 != 0) {
        assert(0 && "worker 0 returned non-zero");
    }

    if( exit_code_1 != 0) {
        assert(0 && "worker 1 returned non-zero");
    }



    // std::vector<uint8_t> gotHighData;
    // gotHighData.assign(((const uint8_t*)gotPacketHigh.data())+1, ((const uint8_t*)gotPacketHigh.data())+gotPacketHigh.size());






    cout << "Parent exit ok\n";
    return 0;

    // cout << "Sleeping\n";
    // for(auto i = 0; i < 1000; i++) {
    //     usleep(1000);
    // }
/*
    std::string header = "fo0";
    // cout << "Header is " << header.size() << "\n";
    // cout << "Header strlen " << strlen(header.c_str()) << "\n";
    zmq::message_t z_header0(header.size()+1);
    memcpy(z_header0.data(), header.c_str(), header.size()+1);

    header = "fo1";
    zmq::message_t z_header1(header.size()+1);
    memcpy(z_header1.data(), header.c_str(), header.size()+1);


    bool ret;
    // ret = worker0.send(z_header0);
    // ret = worker1.send(z_header1);
    // cout << "ret: " << (int)ret << "\n";
    (void)ret;

    uint32_t zmq_poll_interval = 100;//300; // in ms // FIXME what about 10    


    // auto p0 = std::chrono::steady_clock::now();



    // std::vector<std::pair<unsigned,char>> got;

    int sent = 0;


    auto CheckAll = GetDualChecker(0, worker0, worker1,
        [&](const char* const d, const size_t len) {

            cout << "s0 has data " << d << "\n";
            // if( sent < 2 ) {

                    std::string header = "rep" + std::to_string(sent);
                    zmq::message_t replyReplyMsg(header.size()+1);
                    memcpy(replyReplyMsg.data(), header.c_str(), header.size()+1);
                    worker0.send(replyReplyMsg);

                sent++;
            // }
        },
        [&](const char* const d, const size_t len) {
            cout << "s1 has data " << d << "\n";

                    std::string header = "rep" + std::to_string(sent);
                    zmq::message_t replyReplyMsg(header.size()+1);
                    memcpy(replyReplyMsg.data(), header.c_str(), header.size()+1);
                    worker1.send(replyReplyMsg);

                    sent++;

        });


    for(unsigned i = 0; i < 30; i++) {
    // while(1) {
        CheckAll();
    }

    // exit(0);



    // for(auto i = 0; i < 4000; i++) {
    //     usleep(1000);
    // }
    cout << "Parent exit\n";



    //  Connect work threads to client threads via a queue
    // zmq::proxy (static_cast<void*>(clients),
    //             static_cast<void*>(workers),
    //             nullptr);
    return 0;
*/
}











///
/// Included as a self test, however we are not calling this at the moment
///
int SelfTestSerDeser(const int argc,  char** const argv,  char** const env) {

    TB_START_PRE();
    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);
    Port32Out* const cs01 = t->outs["cs01out"];
    Port32In* const cs31 = t->ins["cs31in"];


    const unsigned int ideal0  = (rand()&0xffffffff);
    const unsigned char ideal1 = (rand()&0x1);

    // cout << HEX32_STRING(ideal0) << "\n";
    // cout << (int)(ideal1) << "\n";

    *cs01->i_data = ideal0;
    *cs01->i_valid = ideal1;

    const std::vector<uint8_t> d1 = cs01->Serialize();

    cs31->Deserialize(d1);

    assert(*cs31->t_data == ideal0  && "Assert Failed");
    assert(*cs31->t_valid == ideal1 && "Assert Failed");


    ///////////


    const unsigned char ideal2 = (rand()&0x1);

    *cs31->t_ready = ideal2;


    const std::vector<uint8_t> d2 = cs31->Serialize();

    cs01->Deserialize(d2);

    assert(*cs01->i_ready == ideal2 && "Assert Failed");



    // cout << "All Tests pass\n";




    return 0;
}



















