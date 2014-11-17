//Author Eleftherios(Lef) Kampianakis

//V0: Tx Rx wokrking but not synched
//V1: effort for synching (failed)
//V2: used threads to start tx asynch

#include <uhd/types/tune_request.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <fstream>
#include <complex>
#include <csignal>

//------------------MACROS------------------
#define DEB(x) std::cout << "DEB:" << x << std::endl


//------------------SETUP------------------

//Assign the namespace po from boost::program_options
//This is done to work with the terminal inputs cause boost::program_options
// Question: why use namespace instead of .hpp
namespace po = boost::program_options;



#define CPU_FORMAT "fc32"
#define WIRE_FORMAT "sc16"
#define REF_CLOCK "internal"
#define SAMP_RATE 200e3
#define CENT_FREQ 915e6
#define TX_GAIN  10 //dB
#define RX_GAIN 10   //dB
#define SAMPLES_PER_BUFFER 2000
#define ARGS ""
#define TX_FILENAME "/Users/kampianakis/Desktop/SGCC/Demos/UHD/uhd/host/build/examples/data/sin_IQ_TX"
#define RX_FILENAME "/Users/kampianakis/Desktop/SGCC/Demos/UHD/uhd/host/build/examples/data/sin_IQ_RX"
#define SN std::endl
#define RX_BW SAMP_RATE/2;
#define TX 1
#define RX_CONT 0
#define TOTAL_SAMPLES 1e6
#define SYNCH_DELAY 2 //Seconds
#define DEBUG 0
#define RX_TIMEOUT 3

//Stuff for tx asynch. Declare as global to avoid input in thread (chicken)
uhd::tx_streamer::sptr tx_stream;
uhd::tx_metadata_t tx_md;                                                  //TX metadata structure for describing received IF data. Includes time specification, and start and stop burst flags. The send routines will convert the metadata to IF data headers.        
size_t samples_per_buff = SAMPLES_PER_BUFFER;
size_t num_tx_samps = 0;
std::vector<std::complex<float> > small_tx_buff(samples_per_buff);



void thread_startTx(){
    num_tx_samps = tx_stream->send(&small_tx_buff.front(), small_tx_buff.size(), tx_md);
}




typedef boost::function<uhd::sensor_value_t (const std::string&)> get_sensor_fn_t;
bool check_locked_sensor(std::vector<std::string> sensor_names, const char* sensor_name, get_sensor_fn_t get_sensor_fn, double setup_time){
    if (std::find(sensor_names.begin(), sensor_names.end(), sensor_name) == sensor_names.end())
        return false;

    boost::system_time start = boost::get_system_time();
    boost::system_time first_lock_time;

    std::cout << boost::format("Waiting for \"%s\": ") % sensor_name;
    std::cout.flush();

    while (true){
        if ((not first_lock_time.is_not_a_date_time()) and
            (boost::get_system_time() > (first_lock_time + boost::posix_time::seconds(setup_time))))
        {
            std::cout << " locked." << std::endl;
            break;
        }
        if (get_sensor_fn(sensor_name).to_bool()){
            if (first_lock_time.is_not_a_date_time())
                first_lock_time = boost::get_system_time();
            std::cout << "+";
            std::cout.flush();
        }
        else{
            first_lock_time = boost::system_time(); //reset to 'not a date time'

            if (boost::get_system_time() > (start + boost::posix_time::seconds(setup_time))){
                std::cout << std::endl;
                throw std::runtime_error(str(boost::format("timed out waiting for consecutive locks on sensor \"%s\"") % sensor_name));
            }
            std::cout << "_";
            std::cout.flush();
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    std::cout << std::endl;
    return true;
}



int UHD_SAFE_MAIN(int argc, char *argv[]){
    
    uhd::set_thread_priority_safe();    

    std::string args, tx_file, rx_file, type, ref, wire_format, cpu_format;                
    
    double rate, freq, tx_gain, rx_gain, rx_bw, delay, lo_off,seconds_in_future, rx_timeout;

    rx_bw = RX_BW;
    rx_gain = RX_GAIN;
    wire_format = WIRE_FORMAT;
    cpu_format = CPU_FORMAT;
    rate = SAMP_RATE;
    args = ARGS;
    ref = REF_CLOCK;
    freq = CENT_FREQ;
    tx_gain = TX_GAIN;
    // samples_per_buff = SAMPLES_PER_BUFFER;
    tx_file = TX_FILENAME;
    rx_file = RX_FILENAME;
    seconds_in_future = SYNCH_DELAY;
    rx_timeout = RX_TIMEOUT;

    //------------------INIT TX------------------
                                        //Set the scheduling priority on the current thread. Same as set_thread_priority but does not throw on failure.
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);       //Make the usrp by calling the constructor with param the args

    usrp->set_clock_source(ref);                                          //Set the clock source for the usrp device. This sets the source for a 10 MHz reference clock. Typical options for source: internal, external, MIMO.
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate/1e6) << std::endl;                           
    usrp->set_tx_rate(rate);                                                                                        //Set the sample rate
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp->get_tx_rate()/1e6) << std::endl << std::endl;

    std::cout << boost::format("Setting TX Freq: %f MHz...") % (freq/1e6) << std::endl;                              //Set up tuning frequency
    uhd::tune_request_t tune_request;                                                                               
    tune_request = uhd::tune_request_t(freq);                                                                        //Generate the tune request
    usrp->set_tx_freq(tune_request);                                                                                //Tune to CENT_FREQ
    std::cout << boost::format("Actual TX Freq: %f MHz...") % (usrp->get_tx_freq()/1e6) << std::endl << std::endl;  //PRINT Actual CENT_FREQ

    std::cout << boost::format("Setting TX Gain: %f dB...") % tx_gain << std::endl;                                    
    usrp->set_tx_gain(tx_gain);                                                                                     //Set the tx_gain
    std::cout << boost::format("Actual TX Gain: %f dB...") % usrp->get_tx_gain() << std::endl << std::endl;
    
    //------------------CHECK STUFF------------------
    //Check Ref and LO Lock detect
    std::vector<std::string> sensor_names;
    sensor_names = usrp->get_tx_sensor_names(0);
    if (std::find(sensor_names.begin(), sensor_names.end(), "lo_locked") != sensor_names.end()) {
        uhd::sensor_value_t lo_locked = usrp->get_tx_sensor("lo_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % lo_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(lo_locked.to_bool());
    }        



        

    

        
    //------------------INIT RX------------------

    //IS THIS NECESSARY?
    //always select the subdevice first, the channel mapping affects the other settings
    //usrp->set_rx_subdev_spec(subdev);

    std::cout << boost::format("Setting RX Rate: %f Msps...") % (rate/1e6) << std::endl;
    usrp->set_rx_rate(rate);
    std::cout << boost::format("Actual RX Rate: %f Msps...") % (usrp->get_rx_rate()/1e6) << std::endl << std::endl;

    //set the center frequency
    
    std::cout << boost::format("Setting RX Freq: %f MHz...") % (freq/1e6) << std::endl;
    usrp->set_rx_freq(tune_request);
    std::cout << boost::format("Actual RX Freq: %f MHz...") % (usrp->get_rx_freq()/1e6) << std::endl << std::endl;
    
    
    std::cout << boost::format("Setting RX Gain: %f dB...") % rx_gain << std::endl;
    usrp->set_rx_gain(rx_gain);
    std::cout << boost::format("Actual RX Gain: %f dB...") % usrp->get_rx_gain() << std::endl << std::endl;    

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow 1sec  setup time
    //------------------CHECK STUFF------------------
    
    //Always check for locked sensor
    check_locked_sensor(usrp->get_rx_sensor_names(0), "lo_locked", boost::bind(&uhd::usrp::multi_usrp::get_rx_sensor, usrp, _1, 0), 1);

    //------------------INIT FILES---------------
    

    std::ofstream outfile;    
    outfile.open(rx_file.c_str(), std::ofstream::binary);
    if(!outfile.good()){
        std::cout << "OUT File error\n";
        return 0;
    }

    std::ifstream infile(tx_file.c_str(), std::ifstream::binary);

    if(!infile.good()){
        std::cout << "IN File error\n";
        return 0;
    }

    //------------------INIT STREAMS---------------   

    //Stream ARGS
    uhd::stream_args_t stream_args(cpu_format, wire_format);                //Call the constructor of the class stream_args_t and generate the stream_args object with inputs the cpu_format and wire_format (this is the format per sample)

    tx_stream = usrp->get_tx_stream(stream_args);                           //Generate a tx_streamer object named tx_stream using the usrp->get_tx_stream(stream_args). Remember, usrp is already initialized
    uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);    //Generate a tx_streamer object named tx_stream using the usrp->get_tx_stream(stream_args). Remember, usrp is already initialized




    //Setup metadata    

    //Setup tx_metadata

    tx_md.start_of_burst = true;                                              //Set start of burst to true for the first packet in the chain. ?
    tx_md.end_of_burst =   false;        

    #define SYNCHED_TXRX 1
    
    //For TX
    if(SYNCHED_TXRX){
        tx_md.has_time_spec = true;

    }else{
        tx_md.has_time_spec = false;
    }

    //Setup rx_metadata
    uhd::rx_metadata_t rx_md;

    
    //Setup stream command ONLY FOR RX
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);        
    stream_cmd.num_samps = samples_per_buff;                            




    //For RX
    if(SYNCHED_TXRX){
        stream_cmd.stream_now = false;                                   
        // tx_md.time_spec =   stream_cmd.time_spec;           
    }else{
        stream_cmd.stream_now = true;                                   
        stream_cmd.time_spec = uhd::time_spec_t();  
    }


    if(SYNCHED_TXRX){
        //Cannt get any faster than this
        tx_md.time_spec = stream_cmd.time_spec = uhd::time_spec_t(seconds_in_future)+usrp->get_time_now();         
    }


    //Create the  buffs    
    
    std::vector<std::complex<float> > small_rx_buff(samples_per_buff);

    
    //Fill the  TX buffer
    for (int i = 0; i < samples_per_buff; ++i){
        
        infile.read((char*)&small_tx_buff.at(i), small_tx_buff.size()*sizeof(std::complex<float>));    

    }    
    infile.close();                                                        //Close the file pointer

    
    //Issue the  stream command
    rx_stream->issue_stream_cmd(stream_cmd);

    //Print number of maximum buffer size
    printf("MAX TX: %d\n", (int)tx_stream->get_max_num_samps());
    printf("MAX RX %d\n", (int)rx_stream->get_max_num_samps());


    size_t num_rx_samps = 0;

    

    boost::thread txThread(thread_startTx);
    
    //receivotrnsmit        
    txThread.join();                        //Strart the thread for tx  (tx is f blocking)
    num_rx_samps = rx_stream->recv(&small_rx_buff.front(), small_rx_buff.size(), rx_md, rx_timeout, false);  // Receive buffers containing samples described by the metadata.    

    // num_rx_samps = rx_stream->recv(&small_rx_buff.front(), small_rx_buff.size(), rx_md, rx_timeout, false);  // Receive buffers containing samples described by the metadata.    


    //Wait for everything to stop
    boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
    
    

    //Cleanup and print what happened
    usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS); // Stop the stream (not really necessary here)
    
    double rx_stamp = rx_md.time_spec.get_full_secs() + rx_md.time_spec.get_frac_secs();
    double tx_stamp = tx_md.time_spec.get_full_secs() + tx_md.time_spec.get_frac_secs();
    double t_diff = rx_stamp - tx_stamp;


    printf("RX Time stamp: %.12lf\n ΤX Time stamp: %.12lf\n Diff: %.12lf\n",rx_stamp, tx_stamp, t_diff);
    



    switch ( rx_md.error_code ) {

        case uhd::rx_metadata_t::ERROR_CODE_NONE:
            printf("No error:)\n");
            break;
        case uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
            printf("MDError 2\n");
            break;
        case uhd::rx_metadata_t::ERROR_CODE_LATE_COMMAND:
            printf("MDError 3\n");
            break;
        case uhd::rx_metadata_t::ERROR_CODE_BROKEN_CHAIN:
            printf("MDError 4\n");
            break;
        case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
            printf("MDError 5\n");
            break;
        case uhd::rx_metadata_t::ERROR_CODE_ALIGNMENT:
            printf("MDError 6\n");
            break;
        case uhd::rx_metadata_t::ERROR_CODE_BAD_PACKET:
            printf("MDError 7\n");
            break;

        default:
            printf("WUT\n");
            break;

    }    

    //write the  samples
    if (outfile.is_open()){
        outfile.write((const char*)&small_rx_buff.front(), num_rx_samps*sizeof(std::complex<float>));          
    }



    outfile.close();                                                        //Close the file pointer



    //print
    std::cout << "Transmitted samples: " << num_tx_samps << '\n';
    std::cout << "Received samples: " << num_rx_samps << '\n';




 
    
    return EXIT_SUCCESS;
}











