set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set s1=solid.vert
set s2=solid.frag
set s3=instancing.vert
set s4=instancing.frag

%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv
%glslc% %s3% -o %s3%.spv
%glslc% %s4% -o %s4%.spv
pause