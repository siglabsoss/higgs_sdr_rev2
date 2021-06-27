/*
 * Package: higgs_sdr_global_pkg 
 * 
 * Handy constants and such that find use in mulitple Higgs SDR FPGAs 
 * 
 */
 
`ifndef HIGGS_SDR_GLOBAL_PKG_INCLUDED

    `define HIGGS_SDR_GLOBAL_PKG_INCLUDED

    package higgs_sdr_global_pkg;
        
        // NOTE: HOST PC MAC ADDRESS IS LEARNED FROM ARP REQUESTS SENT FROM THE PC TO THE RADIO
        parameter bit [47:0] LOCAL_MAC_ADDR = {8'h06, 8'h07, 8'h08, 8'h09, 8'h0a, 8'h0b}; // MAC address of Higgs SDR
//        parameter bit [31:0] LOCAL_IP_ADDR  = {8'd192, 8'd168, 8'd2, 8'd2};               // IP address of Higgs SDR
//        parameter bit [31:0] HOST_IP_ADDR   = {8'd192, 8'd168, 8'd2, 8'd1};
        parameter bit [31:0] LOCAL_IP_ADDR  = {8'd10, 8'd2, 8'd2, 8'd2};               // IP address of Higgs SDR
        parameter bit [31:0] HOST_IP_ADDR   = {8'd10, 8'd2, 8'd2, 8'd1};
        
        parameter bit [15:0]   UDP_RX_VERSHA_CAP_TX_PORT = 16'd40001;  // Higgs SDR sends Rx path VershaCapture UDP packets to this port on the host (i.e. host should listen on this port)
    
    endpackage
    
    import higgs_sdr_global_pkg::*;

`endif
