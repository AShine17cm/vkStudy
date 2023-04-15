set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set s1=shader.vert
set s2=shader.frag
set inst1=instancing.vert
set inst2=instancing.frag

%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv
%glslc% %inst1% -o %inst1%.spv
%glslc% %inst2% -o %inst2%.spv
pause