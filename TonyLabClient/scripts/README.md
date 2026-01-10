## TonyLabClient 自动编译运行脚本

### PowerShell（推荐）

在项目根目录执行：

- Debug 编译并运行：
  - `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_run.ps1`
- Release 编译并运行：
  - `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_run.ps1 -Config Release`
- 先清理再重新生成：
  - `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_run.ps1 -Clean -Reconfigure`
- 只编译不运行：
  - `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_run.ps1 -NoRun`

可选参数：
- `-BuildDir <path>`：指定构建目录（默认 `./build`）
- `-Generator <name>`：指定 CMake Generator（例如 `"Visual Studio 17 2022"`、`"Ninja"`）
- `-Parallel <n>`：并行编译线程数
- `-RunArgs <args...>`：传给 `TonyLabClient.exe` 的参数

### CMD 入口

- Debug：`build_run.cmd`
- Release：`build_run.cmd Release`

> 说明：首次运行需要已安装 CMake + Visual Studio Build Tools（或完整 VS），并确保 `cmake` 在 PATH 中。
