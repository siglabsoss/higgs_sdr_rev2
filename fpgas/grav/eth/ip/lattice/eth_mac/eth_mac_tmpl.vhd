--VHDL instantiation template

component eth_mac is
    port (gbit_mac_haddr: in std_logic_vector(7 downto 0);
        gbit_mac_hdatain: in std_logic_vector(7 downto 0);
        gbit_mac_hdataout: out std_logic_vector(7 downto 0);
        gbit_mac_rx_dbout: out std_logic_vector(7 downto 0);
        gbit_mac_rx_stat_vector: out std_logic_vector(31 downto 0);
        gbit_mac_rxd: in std_logic_vector(7 downto 0);
        gbit_mac_tx_fifodata: in std_logic_vector(7 downto 0);
        gbit_mac_tx_sndpaustim: in std_logic_vector(15 downto 0);
        gbit_mac_tx_statvec: out std_logic_vector(30 downto 0);
        gbit_mac_txd: out std_logic_vector(7 downto 0);
        gbit_mac_cpu_if_gbit_en: out std_logic;
        gbit_mac_hclk: in std_logic;
        gbit_mac_hcs_n: in std_logic;
        gbit_mac_hdataout_en_n: out std_logic;
        gbit_mac_hread_n: in std_logic;
        gbit_mac_hready_n: out std_logic;
        gbit_mac_hwrite_n: in std_logic;
        gbit_mac_ignore_pkt: in std_logic;
        gbit_mac_reset_n: in std_logic;
        gbit_mac_rx_dv: in std_logic;
        gbit_mac_rx_eof: out std_logic;
        gbit_mac_rx_er: in std_logic;
        gbit_mac_rx_error: out std_logic;
        gbit_mac_rx_fifo_error: out std_logic;
        gbit_mac_rx_fifo_full: in std_logic;
        gbit_mac_rx_stat_en: out std_logic;
        gbit_mac_rx_write: out std_logic;
        gbit_mac_rxmac_clk: in std_logic;
        gbit_mac_tx_discfrm: out std_logic;
        gbit_mac_tx_done: out std_logic;
        gbit_mac_tx_en: out std_logic;
        gbit_mac_tx_er: out std_logic;
        gbit_mac_tx_fifoavail: in std_logic;
        gbit_mac_tx_fifoctrl: in std_logic;
        gbit_mac_tx_fifoempty: in std_logic;
        gbit_mac_tx_fifoeof: in std_logic;
        gbit_mac_tx_macread: out std_logic;
        gbit_mac_tx_sndpausreq: in std_logic;
        gbit_mac_tx_staten: out std_logic;
        gbit_mac_txmac_clk: in std_logic
    );
    
end component eth_mac; -- sbp_module=true 
_inst: eth_mac port map (gbit_mac_haddr => __,gbit_mac_hdatain => __,gbit_mac_hdataout => __,
            gbit_mac_rx_dbout => __,gbit_mac_rx_stat_vector => __,gbit_mac_rxd => __,
            gbit_mac_tx_fifodata => __,gbit_mac_tx_sndpaustim => __,gbit_mac_tx_statvec => __,
            gbit_mac_txd => __,gbit_mac_cpu_if_gbit_en => __,gbit_mac_hclk => __,
            gbit_mac_hcs_n => __,gbit_mac_hdataout_en_n => __,gbit_mac_hread_n => __,
            gbit_mac_hready_n => __,gbit_mac_hwrite_n => __,gbit_mac_ignore_pkt => __,
            gbit_mac_reset_n => __,gbit_mac_rx_dv => __,gbit_mac_rx_eof => __,
            gbit_mac_rx_er => __,gbit_mac_rx_error => __,gbit_mac_rx_fifo_error => __,
            gbit_mac_rx_fifo_full => __,gbit_mac_rx_stat_en => __,gbit_mac_rx_write => __,
            gbit_mac_rxmac_clk => __,gbit_mac_tx_discfrm => __,gbit_mac_tx_done => __,
            gbit_mac_tx_en => __,gbit_mac_tx_er => __,gbit_mac_tx_fifoavail => __,
            gbit_mac_tx_fifoctrl => __,gbit_mac_tx_fifoempty => __,gbit_mac_tx_fifoeof => __,
            gbit_mac_tx_macread => __,gbit_mac_tx_sndpausreq => __,gbit_mac_tx_staten => __,
            gbit_mac_txmac_clk => __);
