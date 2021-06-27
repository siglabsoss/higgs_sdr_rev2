
/// callbacks that are for a specific tag. no tag is passed as argument, only address and words
typedef std::function<void(const uint32_t, const std::vector<uint32_t>&)> mem_words_tag_cb_t;
/// callbacks that are for a specific tag. no tag is passed as argument, only address and words
typedef std::function<void(const uint32_t, const std::vector<uint8_t>&)> mem_bytes_tag_cb_t;
/// callbacks that are for any tag, thus tag, address, words are passed
typedef std::function<void(const uint8_t, const uint32_t, const std::vector<uint32_t>&)> vmem_imem_uart_cb_t;
typedef std::pair<uint8_t, mem_words_tag_cb_t> tag_word_pair_cb_t;
typedef std::pair<uint8_t, mem_bytes_tag_cb_t> tag_byte_pair_cb_t;
typedef std::function<void(const uint8_t, const uint32_t, const uint32_t)> parse_p_len_t; // internal callback type

class SerialParser {
public:
  unsigned char *line_in;
  unsigned char *line_out;
  unsigned char value;
  unsigned start_bit = 0;
  unsigned UART_START_BIT_LEN = 0;
  unsigned UART_PKT_LEN = 0;
  std::string fpga = "";
  bool enable = true;
  std::vector<uint8_t> _frame;
  int _length_counter = 0;
  int _length = 0;
  bool print_missing_callback = true;
  bool print = false;
  bool print2 = false;
  bool print_chars = false;
  bool debug_printf = false;
  std::vector<uint8_t> data;
  std::vector<uint8_t> print_history;
  constexpr static uint8_t max_valid_type = 8; // inclusive

  SerialParser() {
    // cout << "SerialParser()\n";
    speed(0);
  }

  void speed(const unsigned choice) {
    if( choice == 0 ) {
        UART_START_BIT_LEN = (5);
        UART_PKT_LEN = (9);
    }
  }
  
  uint8_t getByte(std::vector<uint8_t> *frame) {
    uint8_t word = frame->front();
    frame->erase(frame->begin());
    return word;
  }

  uint32_t getWord(std::vector<uint8_t> *frame) {
    uint32_t word = getByte(frame);
    word |= getByte(frame) << 8;
    word |= getByte(frame) << 16;
    word |= getByte(frame) << 24;
    return word;
  }

  uint16_t getU16(std::vector<uint8_t> *frame) {
    uint16_t word = getByte(frame);
    word |= getByte(frame) << 8;
    return word;
  }
  // std::vector<uart_cb_t> uart_callbacks;

  std::vector<vmem_imem_uart_cb_t> vmem_uart_callbacks;

  std::vector<tag_word_pair_cb_t> vmem_tag_uart_callbacks;
  std::vector<tag_word_pair_cb_t> imem_word_tag_uart_callbacks;
  std::vector<tag_byte_pair_cb_t> imem_byte_tag_uart_callbacks;

  std::function<std::vector<uint32_t>(const std::string, const unsigned, const unsigned)> get_vmem_data;
  std::function<std::vector<uint32_t>(const std::string, const unsigned, const unsigned)> get_imem_data;

  ///
  /// Pass a tag and a filename prefix
  /// we will write a new file each time we get the correct tag
  void vmemTagToHexFile(const uint8_t _tag, const std::string base, const bool _print = true) {


    // inorder to make this happen, we allocate a new unsigned int
    // this is a "local variable" to this lamba
    // this variable is captured by value
    // the next time vmemTagToHexFile() is called, a new pointer will be allocated
    // I tried to use a static variable inside the lamba, but multiple calls to 
    // vmemTagToHexFile() shared this value
    // note that we never free this value, but that's ok as it should be here
    // for the entire life of the program
    unsigned* counter = new unsigned();

    constexpr unsigned digits = 3;

    registerVmemImemTagCb([=](const uint32_t addr, const std::vector<uint32_t>& data) mutable {
        std::string numbers = std::to_string(*counter);

        while(numbers.size() < digits) {
            numbers = "0" + numbers;
        }

        const std::string fname = base + numbers + ".hex";
        // cout << "fname: " << fname << "\n";

        file_dump_vec(data, fname, _print);

        // cout << fname << " repeat: " << counter << "\n";
        (*counter)++;
    }, _tag);
  }

  void registerVmemImemCb(vmem_imem_uart_cb_t cb) {
    vmem_uart_callbacks.push_back(cb);
  }

  void registerVmemImemTagCb(mem_words_tag_cb_t cb, const uint8_t tag) {
    //std::cout << "register tag: " << int(tag) << std::endl;
    vmem_tag_uart_callbacks.push_back(tag_word_pair_cb_t(tag,cb));
  }

  void registerImemWordsTagCb(mem_words_tag_cb_t cb, const uint8_t tag) {
    //std::cout << "register tag: " << int(tag) << std::endl;
    imem_word_tag_uart_callbacks.push_back(tag_word_pair_cb_t(tag,cb));
  }

  void registerImemBytesTagCb(mem_bytes_tag_cb_t cb, const uint8_t tag) {
    //std::cout << "register tag: " << int(tag) << std::endl;
    imem_byte_tag_uart_callbacks.push_back(tag_byte_pair_cb_t(tag,cb));
  }



  void jamFrame(std::vector<uint8_t> *frame) {
    if( print ) {
        std::cout << fpga << " JAM FRAME" << "\n";
    }
  }

  void charFrame(std::vector<uint8_t> *frame) {
    uint8_t len;
    std::string s;
    len = getByte(frame);
    s.append(frame->begin(), frame->begin() + len -2);
    frame->erase(frame->begin(), frame->begin() + len -2);
    if( print ) {
        std::cout << fpga << " UART msg: " << s << "\n";
    }
  }

  void literalFrame(std::vector<uint8_t> *frame) {
    getByte(frame);
    if( print ) {
        std::cout << fpga << " Literal frame received" << "\n";
    }
  }

  // std::vector<uint32_t> get_mem_data_test(uint32_t addr, uint32_t len) {
  //   std::vector<uint32_t> data;
  //   for(int i = 0; i < 10; i++) {
  //     data.push_back(i);
  //   }
  //   return data;
  // }

  // static void test_cb(uint8_t tag, uint32_t addr, std::vector<uint32_t> *data) {
  //   std::cout << "TAG: " << int(tag) << " ADDR: " << HEX32_STRING(  int(addr) ) << "\n";
  //   for(auto& it : *data) {
  //     std::cout << int(it) << "\n";
  //   }
  // }

  void vmemPtrFrame(const uint8_t tag, const uint32_t addr, const uint32_t ptr_len) {
    std::vector<uint32_t> data = get_vmem_data(fpga, addr, ptr_len);
    unsigned calls = 0;
    for(auto& cb : vmem_uart_callbacks) {
      cb(tag, addr, data);
      calls++;
    }
    // cout << "here2\n";
    for(const auto row : vmem_tag_uart_callbacks) {
      uint8_t cb_tag;
      mem_words_tag_cb_t cb;
      std::tie(cb_tag,cb) = row;
      //std::cout << int(cb_tag) << " : " << int(tag) << "\n";
      if( cb_tag == tag ) {
       cb(addr, data);
       calls++;
      }
    }

    if( print_missing_callback && calls == 0) {
        std::cout << fpga << " sent " << (int)tag << ", " << HEX32_STRING(addr) << " but no callback was fired" << "\n";
    }
  }


  /// We can only read imem in words
  /// This function must check if the address is not a multiple of 4.  in this case we
  /// need to move thing around in order to get words out correctly
  void imemWordsPtrFrame(const uint8_t tag, const uint32_t addr, const uint32_t _ptr_len) {

    uint32_t ptr_len = _ptr_len;

    uint32_t word_addr = addr/4;
    uint32_t remainder = addr - (word_addr*4);

    if( remainder != 0 ) {
        ptr_len++;
    }

    std::vector<uint32_t> data = get_imem_data(fpga, addr/4, ptr_len);
    unsigned calls = 0;

    if( remainder != 0 ) {
        data = offsetWords(data, _ptr_len, remainder);
    }

    for(const auto row : imem_word_tag_uart_callbacks) {
      uint8_t cb_tag;
      mem_words_tag_cb_t cb;
      std::tie(cb_tag,cb) = row;
      // std::cout << "here " << int(cb_tag) << " : " << int(tag) << "\n";
      if( cb_tag == tag ) {
       cb(addr, data);
       calls++;
      }
    }

    if( print_missing_callback && calls == 0) {
        std::cout << fpga << " sent " << (int)tag << ", " << HEX32_STRING(addr) << " but no callback was fired" << "\n";
    }
  }

  std::vector<uint8_t> getImemChars(const uint32_t addr, const uint32_t ptr_len) {
    uint32_t word_addr = addr/4;
    uint32_t remainder = addr - (word_addr*4);

    // if( remainder != 0 ) {
    //     ptr_len++;
    // }

    std::vector<uint32_t> data = get_imem_data(fpga, addr/4, (ptr_len/4)+1 );

    // if( remainder != 0 ) {
    //     cout << "remainder " << remainder << "\n";
    //     // data = offsetWords(data, _ptr_len, remainder);
    // }

    auto chars = wordsIntoChars(data, ptr_len, remainder);
    return chars;
  }

  /// We can only read imem in words
  /// This function must check if the address is not a multiple of 4.  in this case we
  /// need to move thing around in order to get words out correctly
  void imemBytesPtrFrame(const uint8_t tag, const uint32_t addr, const uint32_t ptr_len) {
    
    auto chars = getImemChars(addr, ptr_len);
    // for(const auto c : data ) {
    //     cout << HEX32_STRING((int)c) << "\n";
    // }
    // cout << "----\n";

    // std::vector<uint8_t> chars;


    // return;


    unsigned calls = 0;
    for(const auto row : imem_byte_tag_uart_callbacks) {
      uint8_t cb_tag;
      mem_bytes_tag_cb_t cb;
      std::tie(cb_tag,cb) = row;
      // std::cout << "here " << int(cb_tag) << " : " << int(tag) << "\n";
      if( cb_tag == tag ) {
       cb(addr, chars);
       calls++;
      }
    }

    if( print_missing_callback && calls == 0) {
        std::cout << fpga << " sent " << (int)tag << ", " << HEX32_STRING(addr) << " but no callback was fired" << "\n";
    }

  }

  /// We can only read imem in words
  /// This function must check if the address is not a multiple of 4.  in this case we
  /// need to move thing around in order to get words out correctly
  void imemBytesPrintf(const uint8_t tag, const uint32_t addr, const uint32_t __ptr_len) {
    const uint32_t _ptr_len = __ptr_len/4;
    uint32_t ptr_len = _ptr_len;

    uint32_t word_addr = addr/4;
    uint32_t remainder = addr - (word_addr*4);

    if( remainder != 0 ) {
        ptr_len++;
    }

    std::vector<uint32_t> data = get_imem_data(fpga, addr/4, ptr_len);
    unsigned calls = 0;

    if( remainder != 0 ) {
        data = offsetWords(data, _ptr_len, remainder);
    }

    const uint32_t format_ptr = data[0];

    const uint32_t riscv_found_percent = data[1]; // how many % did riscv find?

    if( debug_printf ) {
        cout << "----\n";
        for(const auto c : data ) {
            cout << HEX32_STRING(c) << "\n";
        }
        cout << "----\n";
        cout << "\n\n";
    }

    // for ease, we could actually remove this if we fetched 1 by 1 and checked for null
    constexpr unsigned max_format_len = 1024;
    auto format = getImemChars(format_ptr, max_format_len);

    // unsigned fmt_len = 0;
    bool end_found = false;
    for(unsigned i = 0; i < format.size(); i++) {
        if( format[i] == '\0' ) {
            format.resize(i+1); // +1 will include null
            end_found = true;
        }
    }

    if( !end_found ) {
        cout << "warning format had no end before " << max_format_len << "\n";
        return;
    }

    // check for %% until
    uint32_t tb_found_percent = 0;
    int last_p = -1;
    for(unsigned i = 0; i < format.size(); i++) {
        if( format[i] == '%' ) {
            // cout << "last_p " << last_p << "\n";
            if( last_p == ((signed)i)-1 && (last_p != -1) ) {
                cout << "WARNING do not use %% as we don't support it and this may crash riscv\n";
            }
            last_p = (int)i;
            tb_found_percent++;
        }
    }

    if( riscv_found_percent != tb_found_percent ) {
        cout << "WARNING riscv found " << riscv_found_percent << " while tb found " << tb_found_percent << " counts of % sign\n";
        return;
    }

    if( debug_printf ) {
        cout << "format.size() " << format.size() << "\n";
    }

    auto handle = [&](const std::vector<uint8_t>& _format, const unsigned start, const unsigned end, const unsigned parsed) {
        char specifier = _format[end-1];

        if( debug_printf ) {
            cout << "\nmatch:\n";
        }

        std::vector<uint8_t> arg0;

        for(unsigned k = start; k < end; k++) {
            if( debug_printf ) {
                cout << (int)_format[k] << ": " << _format[k] << "\n";
            }
            arg0.push_back(_format[k]);
        }

        arg0.push_back(0); // push on null

        const char* _arg0 = (const char*)arg0.data();

        if( debug_printf ) {
            cout << "formatting: %" << specifier << "\n";
        }

        bool pointer = false;
        switch(specifier) {
            case 's':
                pointer = true;
                break;
            default:
            break;
        }

        int retlen = 0;
        std::vector<uint8_t> buf;
        buf.resize(2048);
        char* const  _buf = (char*)buf.data();

        if( pointer ) {
            std::vector<uint8_t> memory = getImemChars(data[parsed+2], max_format_len);
            const char* format_cc = (const char*)memory.data();
            // cout << format_cc;
            // printf(_arg0, format_cc);
            retlen = snprintf(_buf, buf.size(), _arg0, format_cc);
        } else {
            // printf(_arg0, data[parsed+2]);
            retlen = snprintf(_buf, buf.size(), _arg0, data[parsed+2]);
        }

        printf("%s", _buf);
        if( retlen >= buf.size() ) {
            // if you see this it's probably an error, otherwise increase the 2048 above
            cout << "\nSerialParser.hpp trimmed end of printf " << retlen << "\n";
        }
        for(const auto w : buf) {
            print_history.push_back(w);
        }

    };

    unsigned j;
    unsigned parsed = 0;

    for(unsigned i = 0; i < format.size();) {
        // cout << "i: " << i << "\n";
        for(j = i; j < format.size(); j++) {
            if( debug_printf ) {
                cout << "j: " << j << "\n";
            }
            if( format[j] == '%' ) {
                unsigned start = i;
                unsigned end = j+2;

                handle(format, start, end, parsed);

                parsed++;

                i = j+2;
                break;
            }
        }
        if( j == format.size() ) {
            if( debug_printf ) {
                cout << "\n\ntail " << i << " " << j << "\n";

                for(unsigned k = i; k < format.size(); k++) {
                    cout << (int)format[k] << ": " << format[k] << "\n";
                }
            }

            std::vector<uint8_t> argtail;
            for(unsigned k = i; k < format.size(); k++) {
                argtail.push_back(format[k]);
            }
            argtail.push_back(0); // push on null
            const char* _argtail = (const char*) argtail.data();

            int retlen2 = 0;
            std::vector<uint8_t> buf2;
            buf2.resize(2048);
            char* const  _buf2 = (char*)buf2.data();

            // printf("%s", _argtail);
            retlen2 = snprintf(_buf2, buf2.size(), "%s", _argtail);
            printf("%s", _buf2);

            if( retlen2 >= buf2.size() ) {
                // if you see this it's probably an error, otherwise increase the 2048 above
                cout << "\nSerialParser.hpp (2) trimmed end of printf " << retlen2 << "\n";
            }
            for(const auto w : buf2) {
                print_history.push_back(w);
            }


            break;
        }
    }

    return;
  }

  void parsePointerLengthType(const uint8_t type, std::vector<uint8_t> *frame, parse_p_len_t cb) {
    const uint8_t len = getByte(frame);
    constexpr int expected_len = 7;
    //vmem_uart_callbacks.push_back(&test_cb);
    if(len != expected_len) {
      std::cout << fpga << " UART frame length error - expected: " << expected_len << " actual: " << int(len) << "\n";
      return;
    }
    const uint8_t tag = getByte(frame);
    if( print ) {
        std::cout << fpga << " TYPE: " << int(type) << "\n";
        std::cout << fpga << " TAG:  " << int(tag) << "\n";
    }
    const uint32_t addr = getU16(frame);
    if( print ) {
        std::cout << fpga << " ADDR: " << HEX32_STRING(addr) << "\n";
    }
    const uint32_t ptr_len = getU16(frame)+1;
    if( print || print2 ) {
        std::cout << int(tag) << " " << HEX32_STRING(addr) << " " << ptr_len << "\n";
    }

    cb(tag, addr, ptr_len);
  }



  void datapathFrame(std::vector<uint8_t> *frame) {
    getByte(frame);
    if( print ) {
        std::cout << fpga << " Datapath frame received" << "\n";
    }
  }

  void triggerFrame(std::vector<uint8_t> *frame) {
    getByte(frame);
    if( print ) {
        std::cout << fpga << " Trigger frame received" << "\n";
    }
  }

  
  int parseFrame(std::vector<uint8_t> *frame) {
    if( !enable ) {
        return 0;
    }
    const uint8_t type = getByte(frame);

    switch(type) {
    case 0: //jam
      break;
    case 1: //char
      charFrame(frame);
      break;
    case 2: //literal
      literalFrame(frame);
      break;
    case 3: // vmem_imem_ptr_frame
      // parsePointerLengthType(frame, [&](const uint8_t a, const uint32_t b, const uint32_t c){vmemPtrFrame(a,b,c);});
      parsePointerLengthType(type, frame, std::bind(&SerialParser::vmemPtrFrame, this, _1, _2, _3));
      // vmemPtrFrame(frame);
      break;
    case 4: // data path frame
      datapathFrame(frame);
      break;
    case 5: //trigger frame
      triggerFrame(frame);
      break;
    case 6:
      parsePointerLengthType(type, frame, std::bind(&SerialParser::imemWordsPtrFrame, this, _1, _2, _3));
      break;
    case 7:
      parsePointerLengthType(type, frame, std::bind(&SerialParser::imemBytesPtrFrame, this, _1, _2, _3));
      break;
    case 8:
      parsePointerLengthType(type, frame, std::bind(&SerialParser::imemBytesPrintf, this, _1, _2, _3));
      break;
    default:
      std::cout << "Frame type invalid: " << (int)type << std::endl;
      return -1;
    }

    return 0;
  }
  
  void parser(uint8_t byte) {

    data.push_back(byte);

    if(_frame.size() > 0) {
      _frame.push_back(byte);
      _length_counter++;
      _length = (_length == 0) ? byte:_length;
      //std::cout << int(byte) << " " << _length << " : " << _length_counter << std::endl;
      if(_length != 0 && _length == _length_counter) {
        parseFrame(&_frame);
        _length = 0;
        _length_counter = 0;
      } 
    } else {
      if(byte == 0) {
        if( print ) {
            std::cout << "JAM Frame recieved" << std::endl;
        }
      } else if(byte <= max_valid_type) {
        _frame.push_back(byte);
        _length_counter++;
      } else {
        if( print ) {
            std::cout << "Invalid frame starting with "
                << (int)byte << " which is larger than " << (int) max_valid_type << "\n";
        }
      }
      //std::cout << int(byte) << std::endl;
    }
  }

  
};