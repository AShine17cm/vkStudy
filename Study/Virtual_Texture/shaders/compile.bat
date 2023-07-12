set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag

set pt1=point.vert
set pt2=point.frag

set s1=sparseresidency.vert
set s2=sparseresidency.frag

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv

%glslc% %pt1% -o %pt1%.spv
%glslc% %pt2% -o %pt2%.spv

%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv
pause