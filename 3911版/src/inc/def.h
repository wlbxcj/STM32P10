//================================================
#define FISCAL_CARD 0x00
#define USER_CARD   0x01
#define CHECK_CARD  0x02

//税控操作错误编号统一定义表
//================================================
//get_register_nb 函数的错误类型定义
#define MOD_GET_REG_NB      0x01
  #define grn_SAM_COMM_ERR          0x0101  //SAM卡座通讯错误
  #define grn_ID_COMM_ERR           0x0102  //ID卡座通讯错误
  #define grn_CMD_LEN_INCORRECT     0x0105  //6700 长度错误
  #define grn_APP_BLOCKED           0x0109  //6985 使用条件不满足（应用被锁定）
  #define grn_REGISTERED_ALREADY    0x0106  //6901 使用条件不满足（已经注册，不能再注册） //INCORRECT_STATUS
  #define grn_INEXISTENT_FILE       0x010B  //6A82 文件没找到
  #define grn_P1_P2_INCORRECT       0x010C  //6A86 P1或P2不正确
  #define grn_INS_INCORRECT         0x010E  //6D00 INS不正确
  #define grn_CLA_INCORRECT         0x010F  //6E00 CLA不正确
  #define grn_APP_BLOCKED_FOREVER   0x0111  //9303 应用被永久锁定
  #define grn_GET_RESPONSE_ERR      0x0120  //取响应数据错误
  #define grn_OTHER_ERR             0x0121  //其他错误
  
//terminal_register 函数的错误类型定义
#define MOD_TERMINAL_REG    0x02
  #define tr_SAM_COMM_ERR           0x0201  //SAM卡座通讯错误
  #define tr_ID_COMM_ERR            0x0202  //ID卡座通讯错误
  #define tr_CMD_LEN_INCORRECT      0x0205  //6700 长度错误
  #define tr_APP_BLOCKED            0x0209  //6985 使用条件不满足（应用被锁定）
  #define tr_REGISTERED_ALREADY     0x0206  //6901 使用条件不满足（已经注册，不能再注册） //INCORRECT_STATUS
  #define tr_INEXISTENT_FILE        0x020B  //6A82 文件没找到
  #define tr_P1_P2_INCORRECT        0x020C  //6A86 P1或P2不正确
  #define tr_INS_INCORRECT          0x020E  //6D00 INS不正确
  #define tr_CLA_INCORRECT          0x020F  //6E00 CLA不正确
  #define tr_APP_BLOCKED_FOREVER    0x0211  //9303 应用被永久锁定
  #define tr_GET_RESPONSE_ERR       0x0220  //取响应数据错误
  #define tr_OTHER_ERR              0x0221  //其他错误
  
//issue_invoice 函数的错误类型定义
#define MOD_ISSUE_INVOICE   0x03
  #define ii_SAM_COMM_ERR           0x0301  //SAM卡座通讯错误
  #define ii_ID_COMM_ERR            0x0302  //ID卡座通讯错误
  #define ii_CMD_LEN_INCORRECT      0x0305  //6700 长度错误
  #define ii_APP_BLOCKED            0x0309  //6985 使用条件不满足（一般不是应用被锁定）
  #define ii_NOT_REGISTERED         0x0306  //6901 不满足命令执行条件（如没有注册）
  #define ii_CMD_DATA_INCORRECT     0x030A  //6A80 数据参数不正确（如日期、税种税目、发票号（打印）错误等）
  #define ii_INEXISTENT_FILE        0x030B  //6A82 文件没找到
  #define ii_P1_P2_INCORRECT        0x030C  //6A86 P1或P2不正确
  #define ii_INEXISTENT_REFERENCE   0x030D  //6A88 引用数据未找到
  #define ii_INS_INCORRECT          0x030E  //6D00 INS不正确
  #define ii_CLA_INCORRECT          0x030F  //6E00 CLA不正确
  #define ii_APP_BLOCKED_FOREVER    0x0311  //9303 应用被永久锁定
  #define ii_EXCEED_MAX_MONEY       0x0312  //9401 金额超限
  #define ii_KEY_VER_NOT_SUPPORT    0x0313  //9403 密钥版本不支持
  #define ii_GET_RESPONSE_ERR       0x0320  //取响应数据错误
  #define ii_OTHER_ERR              0x0321  //其他错误

//declare_duty 函数的错误类型定义
#define MOD_DECLARE_DUTY    0x04
  #define dd_SAM_COMM_ERR           0x0401  //SAM卡座通讯错误
  #define dd_ID_COMM_ERR            0x0402  //ID卡座通讯错误
  #define dd_CARD_MEMORY_ERR        0x0404  //6581 内存错误
  #define dd_CMD_LEN_INCORRECT      0x0405  //6700 长度错误
  #define dd_APP_BLOCKED            0x0409  //6985 使用条件不满足（应用被锁定）
  #define dd_NOT_REGISTERED         0x0406  //6901 不满足命令执行条件（如没有注册等）
  #define dd_CMD_DATA_INCORRECT     0x040A  //6A80 数据参数不正确（如申报日期不对等）
  #define dd_INEXISTENT_FILE        0x040B  //6A82 文件没找到
  #define dd_P1_P2_INCORRECT        0x040C  //6A86 P1或P2不正确
  #define dd_INEXISTENT_REFERENCE   0x040D  //6A88 引用数据未找到
  #define dd_INS_INCORRECT          0x040E  //6D00 INS不正确
  #define dd_CLA_INCORRECT          0x040F  //6E00 CLA不正确
  #define dd_APP_BLOCKED_FOREVER    0x0411  //9303 应用被永久锁定
  #define dd_KEY_VER_NOT_SUPPORT    0x0413  //9403 密钥版本不支持
  #define dd_DATA_XOR_ERR           0x041F  //接收的数据块异或校验和错误
  #define dd_GET_RESPONSE_ERR       0x0420  //取响应数据错误
  #define dd_OTHER_ERR              0x0421  //其他错误
  
//update_control 函数的错误类型定义
#define MOD_UPDATE_CTRL     0x05
  #define uc_SAM_COMM_ERR           0x0501  //SAM卡座通讯错误
  #define uc_ID_COMM_ERR            0x0502  //ID卡座通讯错误
  #define uc_CARD_MEMORY_ERR        0x0504  //6581 内存错误
  #define uc_CMD_LEN_INCORRECT      0x0505  //6700 长度错误
  #define uc_APP_BLOCKED            0x0509  //6985 使用条件不满足（应用被锁定）
  #define uc_NOT_REGISTERED         0x0506  //6901 不满足命令执行条件（如没有注册等）
  #define uc_CMD_DATA_INCORRECT     0x050A  //6A80 数据参数不正确
  #define uc_INEXISTENT_FILE        0x050B  //6A82 文件没找到
  #define uc_P1_P2_INCORRECT        0x050C  //6A86 P1或P2不正确
  #define uc_INS_INCORRECT          0x050E  //6D00 INS不正确
  #define uc_CLA_INCORRECT          0x050F  //6E00 CLA不正确
  #define uc_MAC_INCORRECT          0x0510  //9302 MAC不对
  #define uc_APP_BLOCKED_FOREVER    0x0511  //9303 应用被永久锁定
  #define uc_KEY_VER_NOT_SUPPORT    0x0513  //9403 密钥版本不支持
  #define uc_OTHER_ERR              0x0521  //其他错误
  
//input_invoice_nb 函数的错误类型定义
#define MOD_INPUT_INVO_NB   0x06
  #define iin_SAM_COMM_ERR          0x0601  //SAM卡座通讯错误
  #define iin_ID_COMM_ERR           0x0602  //ID卡座通讯错误
  #define iin_CARD_MEMORY_ERR       0x0604  //6581 内存错误
  #define iin_CMD_LEN_INCORRECT     0x0605  //6700 长度错误
  #define iin_APP_BLOCKED           0x0609  //6985 使用条件不满足（应用被锁定、发票存储段已满等）
  #define iin_NOT_REGISTERED        0x0606  //6901 不满足命令执行条件（如没有注册等）
  #define iin_LOW_SAFETY_LEVEL      0x0607  //6982 安全状态不满足
  #define iin_CMD_DATA_INCORRECT    0x060A  //6A80 数据参数不正确
  #define iin_INEXISTENT_FILE       0x060B  //6A82 文件没找到
  #define iin_P1_P2_INCORRECT       0x060C  //6A86 P1或P2不正确
  #define iin_INS_INCORRECT         0x060E  //6D00 INS不正确
  #define iin_CLA_INCORRECT         0x060F  //6E00 CLA不正确
  #define iin_APP_BLOCKED_FOREVER   0x0611  //9303 应用被永久锁定
  #define iin_OTHER_ERR             0x0621  //其他错误

//verify_fiscal_pin 函数的错误类型定义
#define MOD_VERIFY_FSC_PIN  0x07
  #define vfp_SAM_COMM_ERR          0x0701  //SAM卡座通讯错误
  #define vfp_ID_COMM_ERR           0x0702  //ID卡座通讯错误
  #define vfp_FISCAL_PIN_ERR        0x0703  //63CX 校验失败，X表示允许重试的次数
  #define vfp_CMD_LEN_INCORRECT     0x0705  //6700 长度错误
  #define vfp_LOW_SAFETY_LEVEL      0x0707  //6982 安全状态不满足
  #define vfp_PIN_BLOCKED           0x0708  //6983 认证方法(PIN)锁定
  #define vfp_INEXISTENT_REFERENCE  0x070D  //6A88 密钥未找到
  #define vfp_GET_RESPONSE_ERR      0x0720  //取响应数据错误
  #define vfp_OTHER_ERR             0x0721  //其他错误

//daily_collect_sign 函数的错误类型定义
#define MOD_DAILY_COLLECT   0x08
  #define dcs_SAM_COMM_ERR          0x0801  //SAM卡座通讯错误
  #define dcs_ID_COMM_ERR           0x0802  //ID卡座通讯错误
  #define dcs_CMD_LEN_INCORRECT     0x0805  //6700 长度错误
  #define dcs_APP_BLOCKED           0x0809  //6985 使用条件不满足（应用被锁定）
  #define dcs_NOT_REGISTERED        0x0806  //6901 不满足命令执行条件（如没有注册等）
  #define dcs_CMD_DATA_INCORRECT    0x080A  //6A80 数据参数不正确（如日期不对、异或和校验错误等）
  #define dcs_INEXISTENT_FILE       0x080B  //6A82 文件没找到
  #define dcs_P1_P2_INCORRECT       0x080C  //6A86 P1或P2不正确
  #define dcs_INS_INCORRECT         0x080E  //6D00 INS不正确
  #define dcs_CLA_INCORRECT         0x080F  //6E00 CLA不正确
  #define dcs_APP_BLOCKED_FOREVER   0x0811  //9303 应用被永久锁定
  #define dcs_SIGN_TYPE_ERR         0x081E  //用于签字的数据类型错误
  #define dcs_DATA_XOR_ERR          0x081F  //接收的数据块异或校验和错误
  #define dcs_GET_RESPONSE_ERR      0x0820  //取响应数据错误
  #define dcs_OTHER_ERR             0x0821  //其他错误
  
//register_sign 函数的错误类型定义
#define MOD_REGISTER_SIGN   0x09
  #define rs_SAM_COMM_ERR           0x0901  //SAM卡座通讯错误
  #define rs_ID_COMM_ERR            0x0902  //ID卡座通讯错误
  #define rs_CARD_MEMORY_ERR        0x0904  //6581 内存错误
  #define rs_CMD_LEN_INCORRECT      0x0905  //6700 长度错误
  #define rs_APP_BLOCKED            0x0909  //6985 使用条件不满足（应用被锁定）
  #define rs_CMD_DATA_INCORRECT     0x090A  //6A80 数据参数不正确（税控卡编号错误等）
  #define rs_INEXISTENT_FILE        0x090B  //6A82 文件没找到
  #define rs_P1_P2_INCORRECT        0x090C  //6A86 P1或P2不正确
  #define rs_INEXISTENT_REFERENCE   0x090D  //6A88 引用数据未找到
  #define rs_INS_INCORRECT          0x090E  //6D00 INS不正确
  #define rs_CLA_INCORRECT          0x090F  //6E00 CLA不正确
  #define rs_APP_BLOCKED_FOREVER    0x0911  //9303 应用被永久锁定
  #define rs_KEY_VER_NOT_SUPPORT    0x0913  //9403 密钥版本不支持
  #define rs_GET_RESPONSE_ERR       0x0920  //取响应数据错误
  #define rs_OTHER_ERR              0x0921  //其他错误

//data_collect 函数的错误类型定义
#define MOD_DATA_COLLECT    0x0A
  #define udc_SAM_COMM_ERR          0x0A01  //SAM卡座通讯错误
  #define udc_ID_COMM_ERR           0x0A02  //ID卡座通讯错误
  #define udc_CARD_MEMORY_ERR       0x0A04  //6581 内存错误
  #define udc_CMD_LEN_INCORRECT     0x0A05  //6700 长度错误
  #define udc_APP_BLOCKED           0x0A09  //6985 使用条件不满足（应用被锁定）
  #define udc_COLLECTED_ALREADY     0x0A06  //6901 不满足命令执行条件（如已经汇总等）
  #define udc_CMD_DATA_INCORRECT    0x0A0A  //6A80 数据参数不正确（如税控卡编号错误）
  #define udc_INEXISTENT_FILE       0x0A0B  //6A82 文件没找到
  #define udc_P1_P2_INCORRECT       0x0A0C  //6A86 P1或P2不正确
  #define udc_INS_INCORRECT         0x0A0E  //6D00 INS不正确
  #define udc_CLA_INCORRECT         0x0A0F  //6E00 CLA不正确
  #define udc_MAC_INCORRECT         0x0A10  //9302 MAC不对
  #define udc_APP_BLOCKED_FOREVER   0x0A11  //9303 应用被永久锁定
  #define udc_KEY_VER_NOT_SUPPORT   0x0A13  //9403 密钥版本不支持
  #define udc_OTHER_ERR             0x0A21  //其他错误

//distribute_invoice_nb 函数的错误类型定义
#define MOD_DISTRIBUTE      0x0B
  #define ud_SAM_COMM_ERR           0x0B01  //SAM卡座通讯错误
  #define ud_ID_COMM_ERR            0x0B02  //ID卡座通讯错误
  #define ud_CARD_MEMORY_ERR        0x0B04  //6581 内存错误
  #define ud_CMD_LEN_INCORRECT      0x0B05  //6700 长度错误
  #define ud_APP_BLOCKED            0x0B09  //6985 使用条件不满足（应用被锁定等）
  #define ud_INEXISTENT_FILE        0x0B0B  //6A82 文件没找到
  #define ud_P1_P2_INCORRECT        0x0B0C  //6A86 P1或P2不正确
  #define ud_INS_INCORRECT          0x0B0E  //6D00 INS不正确
  #define ud_CLA_INCORRECT          0x0B0F  //6E00 CLA不正确
  #define ud_APP_BLOCKED_FOREVER    0x0B11  //9303 应用被永久锁定
  #define ud_OUT_OF_INVOICE         0x0B12  //9401 无发票分发
  #define ud_GET_RESPONSE_ERR       0x0B20  //取响应数据错误
  #define ud_OTHER_ERR              0x0B21  //其他错误
//================================================

//verify_pin 函数的错误类型定义
#define MOD_VERIFY_PIN      0x0C
  #define uvp_SAM_COMM_ERR          0x0C01
  #define uvp_ID_COMM_ERR           0x0C02  //VERIFY_PIN(USER_CARD)时通讯错误（ID）
  #define uvp_USER_PIN_ERR          0x0C03  //63CX 校验失败，X表示允许重试的次数
  #define uvp_CMD_LEN_INCORRECT     0x0C05  //6700 长度错误
  #define uvp_KEY_TYPE_ERR          0x0C14  //6981 密钥类型错误
  #define uvp_LOW_SAFETY_LEVEL      0x0C07  //6982 安全状态不满足
  #define uvp_PIN_BLOCKED           0x0C08  //6983 认证方法(PIN)锁定
  #define uvp_INEXISTENT_REFERENCE  0x0C0D  //6A88 密钥未找到
  #define uvp_OTHER_ERR             0x0C21  //其他错误
//================================================

//get_response 函数的错误类型定义
#define MOD_GET_RESPONSE    0x20
  #define resp_SAM_COMM_ERR     0x2001//GET_RESPONSE时SAM卡座通讯错误
  #define resp_ID_COMM_ERR      0x2002//GET_RESPONSE时ID卡座通讯错误
  //#define resp_HAS_MORE         0x2000//61XX 还有'XX'数据可返回
  #define resp_LEN_ERR          0x2005//6700 长度错误（Lc大于卡中应答数据长度）
  #define resp_ACTUAL_LEN       0x2019//6CXX 长度错误（Le不正确，'XX'表示实际长度）
  #define resp_NO_RESPONSE      0x201B//6F00 卡中无数据返回
  #define resp_OTHER_ERR        0x2021//其他错误

//get_challenge 函数的错误类型定义
#define MOD_GET_CHALLENGE   0x21
  #define rand_SAM_COMM_ERR     0x2101//GET_CHALLENGE时SAM卡座通讯错误
  #define rand_ID_COMM_ERR      0x2102//GET_CHALLENGE时ID卡座通讯错误
  #define rand_LEN_ERR          0x2105//6700 长度错误
  #define rand_NOT_SUPPORT      0x2117//6A81 不支持此功能
  #define rand_P1_P2_ERR        0x210C//6A86 参数P1-P2不正确
  #define rand_OTHER_ERR        0x2121//其他错误

//sel_file_by_id 函数的错误类型定义
#define MOD_SEL_FILE_BY_ID  0x22
  #define sel_id_SAM_COMM_ERR   0x2201//SEL_FILE_BY_ID时SAM卡座通讯错误
  #define sel_id_ID_COMM_ERR    0x2202//SEL_FILE_BY_ID时ID卡座通讯错误
  #define sel_id_NOT_SUPPORT    0x2217//6A81 不支持此功能（无MF或应用已锁）
  #define sel_INEXISTENT_FILE   0x220B//6A82 未找到文件
  #define sel_id_P1_P2_ERR      0x220C//6A86 参数P1-P2不正确
  #define sel_id_OTHER_ERR      0x2221//其他错误

//rd_record 函数的错误类型定义
#define MOD_RD_RECORD       0x23
  #define rd_rec_SAM_COMM_ERR   0x2301//RD_RECORD时SAM卡座通讯错误
  #define rd_rec_ID_COMM_ERR    0x2302//RD_RECORD时ID卡座通讯错误
  #define rd_rec_NOT_REC_FILE   0x2314//6981 文件类型错误
  #define rd_rec_LOW_SAFETY_L   0x2307//6982 安全状态不满足
  #define rd_rec_CANNT_EXEC     0x2315//6986 不满足命令执行条件（无当前EF文件）
  #define rd_rec_SAFE_MSG_ERR   0x2316//6988 安全报文数据项不正确
  #define rd_rec_NOT_SUPPORT    0x2317//6A81 不支持此功能
  #define rd_rec_NO_FILE        0x230B//6A82 未找到文件
  #define rd_rec_NOT_FOUND      0x2318//6A83 未找到记录
  #define rd_rec_ACTUAL_LEN     0x2319//6CXX 长度错误（Le不正确，'XX'表示实际长度）
  #define rd_rec_RESPONSE_ERR   0x2320//RD_RECORD时GET_RESPONSE错误
  #define rd_rec_OTHER_ERR      0x2321//其他错误

//rd_binary 函数的错误类型定义
#define MOD_RD_BINARY       0x24
  #define rd_bin_SAM_COMM_ERR   0x2401//RD_BINERY时SAM卡座通讯错误
  #define rd_bin_ID_COMM_ERR    0x2402//RD_BINERY时ID卡座通讯错误
  #define rd_bin_NOT_BIN_FILE   0x2414//6981 不是二进制文件
  #define rd_bin_LOW_SAFETY_L   0x2407//6982 安全状态不满足
  #define rd_bin_SAFE_MSG_ERR   0x2416//6988 安全报文数据项不正确
  #define rd_bin_NOT_SUPPORT    0x2417//6A81 不支持此功能
  #define rd_bin_NO_FILE        0x240B//6A82 未找到文件
  #define rd_bin_PARA_ERR       0x241A//6B00 参数错误（偏移地址超出了文件长度）
  #define rd_bin_ACTUAL_LEN     0x2419//6CXX 长度错误（Le不正确，'XX'表示实际长度）
  #define rd_bin_RESPONSE_ERR   0x2420//RD_BINERY时GET_RESPONSE错误
  #define rd_bin_OTHER_ERR      0x2421//其他错误

//update_record 函数的错误类型定义
#define MOD_UPDATE_RECORD   0x25
  #define updt_SAM_COMM_ERR     0x2501//UPDATE_RECORD时SAM卡座通讯错误
  #define updt_ID_COMM_ERR      0x2502//UPDATE_RECORD时ID卡座通讯错误
  #define updt_CARD_MEM_ERR     0x2504//6581 写EEPROM失败
  #define updt_LEN_ERR          0x2505//6700 长度错误
  #define updt_NOT_REC_FILE     0x2514//6981 当前文件不是记录文件
  #define updt_LOW_SAFETY_L     0x2507//6982 安全状态不满足
  #define updt_SAFE_MSG_ERR     0x2516//6988 安全报文数据项不正确
  #define updt_NOT_SUPPORT      0x2517//6A81 不支持此功能(无MF或MF已锁定)
  #define updt_NO_FILE          0x250B//6A82 未找到文件
  #define updt_NOT_FOUND        0x2518//6A83 未找到记录
  #define updt_LACK_SPACE       0x251C//6A84 文件存储空间不够
  #define updt_OTHER_ERR        0x2521//其他错误

//internal_auth 函数的错误类型定义
#define MOD_INTERNAL_AUTH   0x26
  #define in_au_SAM_COMM_ERR    0x2601//INTERNAL_AUTH时SAM卡座通讯错误
  #define in_au_ID_COMM_ERR     0x2602//INTERNAL_AUTH时ID卡座通讯错误
  #define in_au_LC_ERR          0x2605//6700 长度Lc不正确
  #define in_au_INVALID_STAT    0x2606//6901 状态无效
  #define in_au_LOW_SAFETY_L    0x2607//6982 安全状态不满足
  #define in_au_CANT_USE_METH   0x2609//6985 使用条件不满足
  #define in_au_NO_FILE         0x260B//6A82 未找到文件
  #define in_au_NO_KEY          0x260D//6A88 密钥未找到
  #define in_au_RESPONSE_ERR    0x2620//INTERNAL_AUTH时GET_RESPONSE错误
  #define in_au_OTHER_ERR       0x2621//其他错误

//external_auth 函数的错误类型定义
#define MOD_EXTERNAL_AUTH   0x27
  #define ex_au_SAM_COMM_ERR    0x2701//EXTERNAL_AUTH时SAM卡座通讯错误
  #define ex_au_ID_COMM_ERR     0x2702//EXTERNAL_AUTH时ID卡座通讯错误
  #define ex_au_AUTH_FAIL       0x2703//63CX 认证失败，X表示允许重试的次数
  #define ex_au_CARD_MEM_ERR    0x2704//6581 写EEPROM失败
  #define ex_au_LEN_ERR         0x2705//6700 长度错误
  #define ex_au_INVALID_STAT    0x2706//6901 状态无效
  #define ex_au_LOW_SAFETY_L    0x2707//6982 安全状态不满足
  #define ex_au_METHOD_LOCKED   0x2708//6983 认证方法锁定
  #define ex_au_CANT_USE_METH   0x2709//6985 使用条件不满足
  #define ex_au_NO_FILE         0x270B//6A82 未找到文件
  #define ex_au_NO_KEY          0x270D//6A88 密钥未找到
  #define ex_au_OTHER_ERR       0x2721//其他错误
//================================================


//0x00e1-0x00e9，为底层通讯返回值
//================================================
#define TIMEOUT_ERR         0x00E1//底层通讯 超时错误
#define RECE_PARITY_ERR     0x00E2//底层通讯 接收数据时奇偶校验错误
#define SEND_PARITY_ERR     0x00E3//底层通讯 发送数据时奇偶校验错误
#define NOT_CPU_CARD_ERR    0x00E4//底层通讯 非CPU卡
#define ATR_LEN_ERR         0x00E5//底层通讯 ATR数据长度错误
#define ATR_XOR_ERR         0x00E6//底层通讯 ATR数据校验错误
#define ISO_MODE_ERR        0x00E7//底层通讯 CPU卡命令方式字节错误，即不正确的ISO-MODE值
#define ISO_ACK_ERR         0x00E8//底层通讯 CPU卡应答字节错误
#define hdw_CARD_TYPE_ERR   0x00E9//底层通讯层面的卡类型错误

//其他应用层错误定义
//================================================
#define SAM_ATR_ERR         0x1C01//SAM卡座复位错误
#define ID_ATR_ERR          0x1C02//ID卡座复位错误
#define CARD_TYPE_ERR       0x1C03//不是本操作所需的卡类型
#define ea_SEL_FILE_ERR     0x1C04//进入税控应用时选择文件错误
#define REGISTERED_ALREADY  0x1C05//准备注册时发现已经注册
#define NOT_REGISTERED      0x1C06//校验口令时发现尚未注册
#define NO_UPDATE_INF       0x1C07//没有本机器的监控管理更新信息
#define WRONG_TAX_PAYER_NO  0x1C08//税务管理卡的纳税人代码和税控卡中不一致
#define ISSUE_FAIL          0x1C09//开发票失败（税控卡中无税控码，并不是未取到税控码）
#define MACH_NO_INCORRECT   0x1C0A//校验口令或初始化机器时发现卡中机器编号和税控机中不一致

#define DATA_LEN_ERR        0x1C0B//取响应数据时数据长度和预期不符
#define RANDOM_LEN_ERR      0x1C0C//取随机数时数据长度和预期不符
#define SEL_FILE_ERR        0x1C0D//选择文件错误
#define NO_MATCHES          0x1C0E//用户卡中没有此台机器的发票存储文件标识信息

#define NO_CARD_ERR         0x1C0F//用户卡座没有插卡
#define WRONG_USER_CARD     0x1C10//生成报税数据时发现用户卡和税控卡不对应
//================================================
    
//软件版本结构类型
#define SOFTWARE_VER_LEN       18
typedef struct
{
  uint  ui_mach_type;                    //机器型号
  uchar area_code[2];                    //区号
  uchar major_ver;                       //主版本号
  uchar minor_ver;                       //辅版本号
  uchar industry;                        //行业代号
  uchar modi_datetime[6];                //软件最近修改时间
  uchar modifier[4];                     //修改人简称
  uchar reserved;                        //预留
}struct_software_ver;
//机器出厂数据结构类型
#define PRODUCT_LEN          57

typedef struct
{
  uchar mach_no[8];	                     //机器编号BCD
  uchar produce_date[4];                 //生产日期BCD
  uchar hardware_ver[18];                //硬件版本号ASC
  struct_software_ver  st_software_ver;  //软件版本号
  uchar update_datetime[6];              //机器软件升级时间
  uchar ini_flag;                        //机器初始化时间
  uint  ui_crc;                          //CRC校验和
}struct_product;

/////	
#define  	FCARD_MAX_LEN		264     //flash卡一次读写的最大长度
#define LOAD  0X00
//应答命令
#define OK                       1       //请求能执行的应答
#define ER                       4       //请求不能执行的应答

/**********COMMAND DEFINE  *************/
#define SET_RTC   			0x90	
#define TEST_SRAM			0x91				
#define TEST_RTC   			0x92	
#define TEST_FLASH  		0x93	
#define TEST_EEPROM  		0x94
#define TEST_FSC_CARD  		0x96    //测试小卡座(插入税控卡)	 
#define TEST_USER_CARD  	0x97    //测试大卡座
#define WRITE_MACH_NO 0x98    //写机器出厂数据
#define READ_PRODUCT_INF  	0x99    //读机器出厂数据
#define BAT_CHANGE  	0x9A    //电池切换检查 HXW V1.1 2004-10-12
#define BAT_CHECK  	0x9B    //电池供电检查 HXW V1.1 2004-10-12
#define TEST  	0x9F    

#define CPU_CARD  			0x00
#define FLASH_CARD  	    0x01
#define SLE4442_CARD  		0x02
#define SLE4428_CARD  		0x03
	

/**********ERROR DEFINE  *************/
#define  TIME_ERR  				0x10	//时间数据格式错误
#define  WR_USER_CARD_ERR       0X11 	//写用户卡异常
#define  NOCARD_ERR  			0x12  	//请插入用户卡		
#define  FLASH_ERR     			0x13	//flash异常
#define  RD_USER_CARD_ERR       0X14    //读用户卡异常
#define  FSC_CARD_RST_ERR       0X15    //税控卡复位异常
#define  RD_FSC_CARD_ERR        0X16    //读税控卡异常
#define  USER_CARD_RST_ERR      0X17    //用户卡复位异常
#define  EEPROM_ERR         	0X18    //eeprom异常	
#define  SRAM_ERR         	    0X19    //SRAM异常
#define  WRITE_RTC_ERR        	0X1a    //设置时钟异常
#define  PACK_HEAD_ERR     	    0x1b    //命令包头错误
#define  PACK_LENGTH_ERR    	0x1c    //命令包长度错误
#define  CHK_SUM_ERR            0x1d    //命令包校验错误
#define  INS_ERR             0x1e      //指令测试错误
#define  RS232_ERR             0x1f      //串口通讯错误
#define  COMMAND_ERR        	0x50    //无效命令字
  

