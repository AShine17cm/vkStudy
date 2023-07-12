set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag
set s1=shader.vert
set s2=shader.frag
set v1=sparseresidency.vert
set v2=sparseresidency.frag

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv
%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv

%glslc% %v1% -o %v1%.spv
%glslc% %v2% -o %v2%.spv
pause