set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag
set s1=tex.vert
set s2=tex.frag
set a1=texArray.vert
set a2=texArray.frag
set c1=texCube.vert
set c2=texCube.frag
set d1=tex3D.vert
set d2=tex3D.frag
set inst1=instancing.vert
set inst2=instancing.frag

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv
%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv
%glslc% %a1% -o %a1%.spv
%glslc% %a2% -o %a2%.spv
%glslc% %c1% -o %c1%.spv
%glslc% %c2% -o %c2%.spv
%glslc% %d1% -o %d1%.spv
%glslc% %d2% -o %d2%.spv
%glslc% %inst1% -o %inst1%.spv
%glslc% %inst2% -o %inst2%.spv

pause