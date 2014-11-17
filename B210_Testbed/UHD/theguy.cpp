#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <csignal>
#include <complex>

namespace po = boost::program_options;


static bool stop_signal_called = false;


void sig_int_handler(int){stop_signal_called = true;}


typedef boost::function<uhd::sensor_value_t (const std::string&)>


get_sensor_fn_t;
bool check_locked_sensor(std::vector<std::string> sensor_names, const char* sensor_name, get_sensor_fn_t get_sensor_fn, double setup_time)
{
  if (std::find(sensor_names.begin(), sensor_names.end(), sensor_name) == sensor_names.end())  return false;
  boost::system_time start = boost::get_system_time();
  boost::system_time first_lock_time;
  std::cout << boost::format("Waiting for \"%s\": ") % sensor_name;
  std::cout.flush();
  while (true) {
    if ((not first_lock_time.is_not_a_date_time()) and (boost::get_system_time() > (first_lock_time + boost::posix_time::seconds(setup_time)))){
      std::cout << " locked." << std::endl;
      break;
    }

    if (get_sensor_fn(sensor_name).to_bool()){
      if (first_lock_time.is_not_a_date_time())
        first_lock_time = boost::get_system_time();
      std::cout << "+";
      std::cout.flush();
    }
    else {
      first_lock_time = boost::system_time();  //reset to 'not a date  time' 
      if (boost::get_system_time() > (start + boost::posix_time::seconds(setup_time))){
        std::cout << std::endl;
        throw std::runtime_error(str(boost::format("timed out waiting for consecutive locks on sensor \"%s\"") % sensor_name));
      }      
      std::cout << "_"; std::cout.flush();
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }
  std::cout << std::endl;
  return true;
}

int UHD_SAFE_MAIN(int argc, char *argv[])
{
  uhd::set_thread_priority_safe();
  std::string args = "";
  std::string file1 = "usrp_samples1.dat";
  std::string file2 = "usrp_samples2.dat";
  std::string ant = "TX/RX";
  std::string subdev = "A:A A:B";
  size_t total_num_samps;
  double rate, freq, gain, bw, total_time, setup_time = 1.0;
  double seconds_in_future = 2.5;
  size_t samps_per_buff;
  po::options_description desc("Allowed options");
  desc.add_options()("help", "help message")
    ("nsamps", po::value<size_t>(&total_num_samps)->default_value(2000), "total number of samples to receive")
    ("samps_per_buff", po::value<size_t>(&samps_per_buff)->default_value(2000), "samples per buffer")
    ("rate", po::value<double>(&rate)->default_value(1e6), "rate of incoming samples")
    ("freq", po::value<double>(&freq)->default_value(915e6), "RF center frequency in Hz")
    ("gain", po::value<double>(&gain)->default_value(0), "gain for the RF chain")
  ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  
  if (vm.count("help")){
    std::cout << boost::format("UHD RX samples to file %s") % desc << std::endl;
    return ~0;
  }   

  std::cout << std::endl;
  std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
  

  uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);
  
  usrp->set_clock_source("internal",0);
  usrp->set_rx_subdev_spec(subdev);

  std::cout << boost::format("Setting RX Rate: %f Msps...") % (rate/1e6) << std::endl;
  usrp->set_rx_rate(rate);
  std::cout << boost::format("Actual RX Rate: %f Msps...") % (usrp->get_rx_rate(0)/1e6) << std::endl;
  std::cout << boost::format("Actual RX Rate: %f Msps...") % (usrp->get_rx_rate(1)/1e6) << std::endl << std::endl;

  std::cout << boost::format("Setting RX Freq: %f MHz...") % (freq/1e6) << std::endl;
  usrp->set_rx_freq(freq,0);
  usrp->set_rx_freq(freq,1); //they both seem to need to be set even though they must be the same for B210...
  std::cout << boost::format("Actual RX Freq: %f MHz...") % (usrp->get_rx_freq(0)/1e6) << std::endl;
  std::cout << boost::format("Actual RX Freq: %f MHz...") % (usrp->get_rx_freq(1)/1e6) << std::endl << std::endl;

  std::cout << boost::format("Setting RX Gain: %f dB...") % gain << std::endl;
  usrp->set_rx_gain(gain,0);
  usrp->set_rx_gain(gain,1);  //they both seem to need to be set even though they must be the same for B210...
  std::cout << boost::format("Actual RX Gain: %f dB...") % usrp->get_rx_gain(0) << std::endl;
  std::cout << boost::format("Actual RX Gain: %f dB...") % usrp->get_rx_gain(1) << std::endl << std::endl;
  usrp->set_rx_antenna("RX2",0);
  usrp->set_rx_antenna("RX2",1);


  boost::this_thread::sleep(boost::posix_time::seconds(setup_time)); //allow for some setup time

  check_locked_sensor(usrp->get_rx_sensor_names(0), "lo_locked",
  boost::bind(&uhd::usrp::multi_usrp::get_rx_sensor, usrp, _1, 0), setup_time);

  if (total_num_samps == 0){
    std::signal(SIGINT, &sig_int_handler);
    std::cout << "Press Ctrl + C to stop streaming..." << std::endl;
  }

  unsigned long long num_requested_samples = total_num_samps;
  unsigned long long num_total_samps = 0;
  
  uhd::stream_args_t stream_args1("sc16");
  uhd::stream_args_t stream_args2("sc16");
  
  stream_args1.channels.push_back(0);
  stream_args2.channels.push_back(1);
  
  uhd::rx_streamer::sptr rx_stream1 = usrp->get_rx_stream(stream_args1);
  uhd::rx_streamer::sptr rx_stream2 = usrp->get_rx_stream(stream_args2);
  
  uhd::stream_cmd_t stream_cmd1((num_requested_samples == 0)?
    uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS:
    uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE
  );
  
  uhd::stream_cmd_t stream_cmd2((num_requested_samples == 0)?
    uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS:
    uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE
  );
  
  stream_cmd1.num_samps = num_requested_samples;
  stream_cmd2.num_samps = num_requested_samples;
  
  stream_cmd1.stream_now = false;
  stream_cmd2.stream_now = false;
  
  stream_cmd2.time_spec = stream_cmd1.time_spec = uhd::time_spec_t(seconds_in_future)+usrp->get_time_now();     
   

  rx_stream1->issue_stream_cmd(stream_cmd1);
  rx_stream2->issue_stream_cmd(stream_cmd2);
  
  uhd::rx_metadata_t md1, md2;
  
  std::vector<std::complex<short> > buff1(samps_per_buff);
  std::vector<std::complex<short> > buff2(samps_per_buff);
  
  double timeout = seconds_in_future + 2.; //timeout (delay before receive + padding)
  std::ofstream outfile1, outfile2;
  
  outfile1.open(file1.c_str(), std::ofstream::binary);
  outfile2.open(file2.c_str(), std::ofstream::binary);
  
 

  while(not stop_signal_called and (num_requested_samples != num_total_samps or num_requested_samples == 0)){
    
    boost::system_time now = boost::get_system_time();
    size_t num_rx_samps1, num_rx_samps2;
    size_t n_samps_this_time = samps_per_buff;
    if(num_requested_samples) {
      size_t n_samps_to_finish = num_requested_samples - num_total_samps;
      
      if(n_samps_to_finish<n_samps_this_time) n_samps_this_time = n_samps_to_finish;
    }
    

    num_rx_samps1 = rx_stream1->recv(&buff1.front(), n_samps_this_time, md1, timeout);
    num_rx_samps2 = rx_stream2->recv(&buff2.front(), n_samps_this_time, md2, timeout);
    
    double t1 = md1.time_spec.get_full_secs() + md1.time_spec.get_frac_secs();
    double t2 = md2.time_spec.get_full_secs() + md2.time_spec.get_frac_secs();
    printf("asked for %d samples, got %d and %d with time stamps %.12lf and %.12lf (dif= %.12lf)\n",(int)n_samps_this_time,(int)num_rx_samps1,(int)num_rx_samps2,t1,t2,t1-t2);
    

    timeout = 1.0;
    if (md1.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) printf("md1 timed out\n");
    
    if (md1.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
      throw std::runtime_error(str(boost::format("Unexpected error code 0x%x") % md1.error_code));
    }
    if (md2.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) printf("md2 timed out\n");
    if (md2.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) break;
    
    if (md2.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
      throw std::runtime_error(str(boost::format("Unexpected error code 0x%x") % md2.error_code));
    }
    num_total_samps += num_rx_samps1;
    if (outfile1.is_open())
      outfile1.write((const char*)(&buff1.front()), num_rx_samps1*sizeof(std::complex<short>));
    if (outfile2.is_open())
      outfile2.write((const char*)(&buff2.front()), num_rx_samps2*sizeof(std::complex<short>));
  }
  

  if (outfile1.is_open()) outfile1.close();
  if (outfile2.is_open()) outfile2.close();
  std::cout << std::endl << "Done!" << std::endl << std::endl;
  return EXIT_SUCCESS;
}

