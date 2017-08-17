{
  "targets": [
    {
      "target_name": "node-ctp",
      "sources": ["src/shifctp.cc","src/tools.cc","src/stdafx.cpp","src/uv_mduser.cpp","src/uv_trader.cpp","src/wrap_mduser.cpp","src/wrap_trader.cpp" ],
      "libraries":["../ctp_win64/thostmduserapi.lib","../ctp_win64/thosttraderapi.lib"],
      "include_dirs":["ctp_win64/"]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ],
}


