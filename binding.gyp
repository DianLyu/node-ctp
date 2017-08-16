{
  "targets": [
    {
      "target_name": "shifctp",
      "sources": ["src/defines.cpp", "src/shifctp.cc","src/tools.cc","src/stdafx.cpp","src/uv_mduser.cpp","src/uv_trader.cpp","src/wrap_mduser.cpp","src/wrap_trader.cpp" ],
      "libraries":["../ctp_win64/thostmduserapi.lib","../ctp_win64/thosttraderapi.lib"],
      "include_dirs":["ctp_win64/"]
    }
  ],
}


