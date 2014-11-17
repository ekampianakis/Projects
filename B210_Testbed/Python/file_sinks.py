#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: File Sinks
# Generated: Tue Nov  4 15:21:09 2014
##################################################

from gnuradio import analog
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.wxgui import forms
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import time
import wx
import ctypes
import sys
import os
import socket
import scipy.io as sio
from timeout import timeout
from timeout import TimeoutError




properties_file = '../Data/properties_file.mat'

#Default settings
rx_filename1 = ''
rx_filename2 = ''
tx_filename = ''
tx_gain = 0
samp_rate =  1e6
rx_gain =  0
center_freq = 915e6
mclk = 10e6
num_samples = 0


def wait_for_command(connection):
 while True:
        print 'Waiting for matlab command'
        data = connection.recv(16)
        
        if( str(data).startswith('PR')):
            print 'Command received'
            return
        elif(str(data).startswith('EX')):
            print 'Command received'
            print 'Exit command received..'
            raise KeyboardInterrupt     
            return 
        else:
            print 'Wrong Command'               
            time.sleep(1)
            continue


def reply_to_command(connection,print_msg,reply_msg):
    print print_msg
    connection.sendall(reply_msg)



class file_sinks(gr.top_block):

    def __init__(self):
        # grc_wxgui.top_block_gui.__init__(self, title="File Sinks")
        gr.top_block.__init__(self)
        ##################################################
        # Variables
        ##################################################
        self.tx_gain = tx_gain
        self.samp_rate =  samp_rate
        self.rx_gain =  tx_gain
        self.center_freq = center_freq
        self.mclk = mclk
        self.items = num_samples

        self.tx_filename = tx_filename
        self.rx_filename1 = rx_filename1
        self.rx_filename2 = rx_filename2


        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_source_0 = uhd.usrp_source(
            ",".join(("", "")),
            uhd.stream_args(
                cpu_format="fc32",
                channels=range(2),
            ),
        )
        self.uhd_usrp_source_0.set_clock_rate(self.mclk, uhd.ALL_MBOARDS)
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_source_0.set_center_freq(self.center_freq, 0)
        self.uhd_usrp_source_0.set_gain(self.rx_gain, 0)
        self.uhd_usrp_source_0.set_center_freq(self.center_freq, 1)
        self.uhd_usrp_source_0.set_gain(self.rx_gain, 1)
        self.uhd_usrp_source_0.set_bandwidth(self.samp_rate/2);
        

        

        self.uhd_usrp_sink_0 = uhd.usrp_sink(
            ",".join(("", "")),
            uhd.stream_args(
                cpu_format="fc32",
                channels=range(2),
            ),
        )
        self.uhd_usrp_sink_0.set_clock_rate(self.mclk, uhd.ALL_MBOARDS)
        self.uhd_usrp_sink_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_sink_0.set_center_freq(self.center_freq, 0)
        self.uhd_usrp_sink_0.set_gain(self.tx_gain, 0)
        self.uhd_usrp_sink_0.set_antenna("TX/RX2", 0)
        self.uhd_usrp_sink_0.set_center_freq(self.center_freq, 1)
        self.uhd_usrp_sink_0.set_gain(self.tx_gain, 1)
        self.uhd_usrp_sink_0.set_antenna("TX/RX1", 1)
        self.uhd_usrp_sink_0.set_bandwidth(self.samp_rate/2);
    



        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_gr_complex*1, self.tx_filename, False)
        
        self.blocks_file_sink_0_0 = blocks.file_sink(gr.sizeof_gr_complex*1, self.rx_filename2, False)
        self.blocks_file_sink_0_0.set_unbuffered(False)
        
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_gr_complex*1, self.rx_filename1, False)
        self.blocks_file_sink_0.set_unbuffered(False)
        
        self.analog_const_source_x_0 = analog.sig_source_c(0, analog.GR_CONST_WAVE, 0, 0, 0)


        self.blocks_head_0_0_0 = blocks.head(gr.sizeof_gr_complex*1, self.items)
        self.blocks_head_0_0 = blocks.head(gr.sizeof_gr_complex*1, self.items)
        self.blocks_head_0 = blocks.head(gr.sizeof_gr_complex*1, self.items)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_file_source_0, 0), (self.blocks_head_0, 0))
        self.connect((self.blocks_head_0, 0), (self.uhd_usrp_sink_0, 0))
        self.connect((self.analog_const_source_x_0, 0), (self.uhd_usrp_sink_0, 1))
        self.connect((self.blocks_head_0_0, 0), (self.blocks_file_sink_0, 0))
        self.connect((self.blocks_head_0_0_0, 0), (self.blocks_file_sink_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.blocks_head_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 1), (self.blocks_head_0_0_0, 0))



if __name__ == '__main__':

    #delete previous files
    try:
        os.remove(rx_filename1)
        os.remove(rx_filename2)
    except OSError:
        pass



    mat_contents = sio.loadmat(properties_file)
    tx_filename =  str(mat_contents['tx_filename'][0])
    rx_filename1 =  str(mat_contents['rx_filename1'][0])
    rx_filename2 =  str(mat_contents['rx_filename2'][0])
    host =  str(mat_contents['Host'][0])
    tx_gain =  float(mat_contents['TX_gain'][0][0])
    samp_rate =  float(mat_contents['Fs'][0][0])
    rx_gain =  float(mat_contents['RX_gain'][0][0])
    center_freq = float(mat_contents['F_center'][0][0])
    mclk = float(mat_contents['Mclk'][0][0])
    tcp_port = float(mat_contents['Port'][0][0])
    tx_time = float(mat_contents['T_sig'][0][0])
    num_samples = int(mat_contents['Num_samples'][0][0])

    print 'Fs = ' + repr(samp_rate/1e6)
    print 'TX file = ' + repr(tx_filename)
    print 'MCLK = ' + repr(mclk/1e6)
    print 'Center Freq. = ' + repr(center_freq/1e6)
    print 'RX file 1 = ' + repr(rx_filename1)
    print 'RX file 2 = ' + repr(rx_filename2)
    print 'TX time = ' + repr(tx_time)



    # # Create a TCP/IP socket
    # sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # # Bind the socket to the port
    # server_address = ('localhost', tcp_port)
    # print >>sys.stderr, 'Starting up on %s port %s' % server_address
    # sock.bind(server_address)
    # # Listen for incoming connections
    # sock.listen(1)

    # print >>sys.stderr, 'Waiting for matlab a connection...'
    # connection, client_address = sock.accept()
    # print >>sys.stderr, 'Connection from', client_address



    # try:
    #     wait_for_command(connection)
    #     tb = file_sinks()
    #     reply_to_command(connection,'USRP Initialized, informing Matlab..','MR\n')
    #     while True: 
            
    #             while True:

    #                 #Wait for matlab command to start
    #                 wait_for_command(connection)
                    
    #                 tb.blocks_file_sink_0.close()
    #                 tb.blocks_file_sink_0.open(rx_filename1)

    #                 tb.blocks_file_sink_0_0.close()
    #                 tb.blocks_file_sink_0_0.open(rx_filename2)

    #                 tb.start()
    #                 reply_to_command(connection,'Started reception, Waiting for stop message..','MR\n')
    #                 #Wait for matlab command to stop
                    
    #                 wait_for_command(connection)
    #                 tb.stop()
    #                 tb.wait()
    #                 tb.blocks_file_source_0.seek(0,0)
                    
    #                 reply_to_command(connection,'Stopped reception, Informing matlab..','MR\n')

    # finally:
    #         print 'Interrupt, exiting..'
    #         connection.close()
    #         tb.stop()
    #         tb.wait()



    total = 0
    timeouts = 0


    # Simple
    @timeout(3)
    def execute_me():
        tb = file_sinks()
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()
    
    try:
        execute_me()
    except(TimeoutError):
        pass

    while(1):
        
        try:
            total = total +1
            center_freq = center_freq + samp_rate/2
            execute_me()
            print 'stopped'
            time.sleep(2)
        except(KeyboardInterrupt):
            print total
            print timeouts
            sys.exit(-1)
        except(TimeoutError):
            timeouts = timeouts+1
            print 'Timeout Error'
            pass
        
        
    