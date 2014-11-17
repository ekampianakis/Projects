//
// Copyright 2011-2012,2014 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

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


//Assign the namespace po from boost::program_options
//This is done to work with the terminal inputs cause boost::program_options
// Question: why use namespace instead of .hpp

namespace po = boost::program_options;

//Variable that is used is set by the signal handler below. The signal handler is activated when ctrl-c is pressed
static bool stop_signal_called = false;

//Signal handler function (just a simple function with int input param) 
//Question:why input param since its not used?
void sig_int_handler(int){
    stop_signal_called = true;
}





//Function template (typename same as class) that returns a samp_type.
//So this function takes as inputs: a usrp object, the cpu_format, wire_format and the file pointer and transmits studd from the file. How many?
template<typename samp_type> void send_from_file(
    uhd::usrp::multi_usrp::sptr usrp, 
    const std::string &cpu_format,
    const std::string &wire_format,
    const std::string &file,
    size_t samps_per_buff
){

    //create a transmit streamer    
    uhd::stream_args_t stream_args(cpu_format, wire_format);                //Call the constructor of the class stream_args_t and generate the stream_args object with inputs the cpu_format and wire_format (this is the format per sample)
    uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(stream_args);    //Generate a tx_streamer object named tx_stream using the usrp->get_tx_stream(stream_args). Remember, usrp is already initialized


    uhd::tx_metadata_t md;                                                  //TX metadata structure for describing received IF data. Includes time specification, and start and stop burst flags. The send routines will convert the metadata to IF data headers.        
    md.start_of_burst = false;                                              //Set start of burst to true for the first packet in the chain. ?
    md.end_of_burst = false;
    

    std::vector<samp_type> buff(samps_per_buff);                            //Declare a vector of stuff using just the standart namespace and initialize it using size_t samps_per_buff
    std::ifstream infile(file.c_str(), std::ifstream::binary);              //Generate a  file pointer stream named infile using the filename. The stream is declared as binary

    
    while(not md.end_of_burst and not stop_signal_called){                  //loop until the entire file has been read

        infile.read((char*)&buff.front(), buff.size()*sizeof(samp_type));   //istream& read (char* s, streamsize n); OK. read  buff.size()*sizeof(samp_type) characters from the file and store them in the buff vector (ok that's weird (char*)&buff.front())
        size_t num_tx_samps = infile.gcount()/sizeof(samp_type);            //set the number of transmit samples to the number of characters extracted by the last unformatted input operation performed on the object (e.g. read) / sizeof(samp_type)
        md.end_of_burst = infile.eof();                                     //C++ provides a special function, eof( ), that returns nonzero (meaning TRUE) when there are no more data to be read from an input file stream, and zero (meaning FALSE) otherwise.                   
        tx_stream->send(&buff.front(), num_tx_samps, md);                   //Finally send the first element of the buffer yeah.

    }

    infile.close();                                                         //Close the file pointer
}




int UHD_SAFE_MAIN(int argc, char *argv[]){
    uhd::set_thread_priority_safe();                                        //Set the scheduling priority on the current thread. Same as set_thread_priority but does not throw on failure.

    //variables to be set by po
    std::string args, file, type, ant, subdev, ref, wirefmt;                
    size_t spb;
    double rate, freq, gain, bw, delay, lo_off;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "multi uhd device address args")
        ("file", po::value<std::string>(&file)->default_value("usrp_samples.dat"), "name of the file to read binary samples from")
        ("type", po::value<std::string>(&type)->default_value("short"), "sample type: double, float, or short")
        ("spb", po::value<size_t>(&spb)->default_value(10000), "samples per buffer")
        ("rate", po::value<double>(&rate), "rate of outgoing samples")
        ("freq", po::value<double>(&freq), "RF center frequency in Hz")
        ("lo_off", po::value<double>(&lo_off), "Offset for frontend LO in Hz (optional)")
        ("gain", po::value<double>(&gain), "gain for the RF chain")
        ("ant", po::value<std::string>(&ant), "daughterboard antenna selection")
        ("subdev", po::value<std::string>(&subdev), "daughterboard subdevice specification")
        ("bw", po::value<double>(&bw), "daughterboard IF filter bandwidth in Hz")
        ("ref", po::value<std::string>(&ref)->default_value("internal"), "reference source (internal, external, mimo)")
        ("wirefmt", po::value<std::string>(&wirefmt)->default_value("sc16"), "wire format (sc8 or sc16)")
        ("delay", po::value<double>(&delay)->default_value(0.0), "specify a delay between repeated transmission of file")
        ("repeat", "repeatedly transmit file")
        ("int-n", "tune USRP with integer-n tuning")
    ;
    po::variables_map vm;                                           //This thing has the mappings of all input parameters for the script
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){                                          //If the help parameter was input then print help
        std::cout << boost::format("UHD TX samples from file %s") % desc << std::endl;
        return ~0;
    }

    bool repeat = vm.count("repeat");                               //Set repeat to true if its an input param

    //create a usrp device
    std::cout << std::endl;                                         
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    

    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args); //Call the make function of the class uhd::usrp::multi_usrp::sptr to generate and save a usrp object 

    //Lock mboard clocks
    usrp->set_clock_source(ref);

    //always select the subdevice first, the channel mapping affects the other settings
    if (vm.count("subdev")) usrp->set_tx_subdev_spec(subdev);

    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

    //set the sample rate
    if (not vm.count("rate")){
        std::cerr << "Please specify the sample rate with --rate" << std::endl;
        return ~0;
    }
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate/1e6) << std::endl;
    usrp->set_tx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp->get_tx_rate()/1e6) << std::endl << std::endl;

    //set the center frequency
    if (not vm.count("freq")){
        std::cerr << "Please specify the center frequency with --freq" << std::endl;
        return ~0;
    }
    std::cout << boost::format("Setting TX Freq: %f MHz...") % (freq/1e6) << std::endl;
    uhd::tune_request_t tune_request;
    if(vm.count("lo_off")) tune_request = uhd::tune_request_t(freq, lo_off);
    else tune_request = uhd::tune_request_t(freq);
    if(vm.count("int-n")) tune_request.args = uhd::device_addr_t("mode_n=integer");
    usrp->set_tx_freq(tune_request);
    std::cout << boost::format("Actual TX Freq: %f MHz...") % (usrp->get_tx_freq()/1e6) << std::endl << std::endl;

    //set the rf gain
    if (vm.count("gain")){
        std::cout << boost::format("Setting TX Gain: %f dB...") % gain << std::endl;
        usrp->set_tx_gain(gain);
        std::cout << boost::format("Actual TX Gain: %f dB...") % usrp->get_tx_gain() << std::endl << std::endl;
    }

    //set the IF filter bandwidth
    if (vm.count("bw")){
        std::cout << boost::format("Setting TX Bandwidth: %f MHz...") % bw << std::endl;
        usrp->set_tx_bandwidth(bw);
        std::cout << boost::format("Actual TX Bandwidth: %f MHz...") % usrp->get_tx_bandwidth() << std::endl << std::endl;
    }

    //set the antenna
    if (vm.count("ant")) usrp->set_tx_antenna(ant);

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow for some setup time


    // END OF INITS LETS DO SOME WORK

    //Check Ref and LO Lock detect
    std::vector<std::string> sensor_names;
    sensor_names = usrp->get_tx_sensor_names(0);
    if (std::find(sensor_names.begin(), sensor_names.end(), "lo_locked") != sensor_names.end()) {
        uhd::sensor_value_t lo_locked = usrp->get_tx_sensor("lo_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % lo_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(lo_locked.to_bool());
    }
    sensor_names = usrp->get_mboard_sensor_names(0);
    if ((ref == "mimo") and (std::find(sensor_names.begin(), sensor_names.end(), "mimo_locked") != sensor_names.end())) {
        uhd::sensor_value_t mimo_locked = usrp->get_mboard_sensor("mimo_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % mimo_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(mimo_locked.to_bool());
    }
    if ((ref == "external") and (std::find(sensor_names.begin(), sensor_names.end(), "ref_locked") != sensor_names.end())) {
        uhd::sensor_value_t ref_locked = usrp->get_mboard_sensor("ref_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % ref_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(ref_locked.to_bool());
    }


    


    //set sigint if user wants to receive
    if(repeat){
        std::signal(SIGINT, &sig_int_handler);
        std::cout << "Press Ctrl + C to stop streaming..." << std::endl;
    }

    //send from file
    do{
        if (type == "double")send_from_file<std::complex<double> >(usrp, "fc64", wirefmt, file, spb);
        else if (type == "float") send_from_file<std::complex<float> >(usrp, "fc32", wirefmt, file, spb);
        else if (type == "short") send_from_file<std::complex<short> >(usrp, "sc16", wirefmt, file, spb);
        else throw std::runtime_error("Unknown type " + type);

        if(repeat and delay != 0.0) boost::this_thread::sleep(boost::posix_time::milliseconds(delay));
    } while(repeat and not stop_signal_called);

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
