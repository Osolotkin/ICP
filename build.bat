@cl .\src\*.cpp /W0 /wd4530 /I"./include" /Fe"app" /link /LIBPATH:"./lib" opencv_world490.lib glfw3dll.lib glew32.lib glew32s.lib assimp-vc143-mt.lib | Findstr /v "note:" 
@del .\*.obj
