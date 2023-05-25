set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe

set f1=filterCube.vert
set f2=prefilterenvmap.frag

set i1=irradiancecube.frag

set b1=genbrdflut.vert
set b2=genbrdflut.frag

set p1=pbr.vert
set p2=pbr_khr.frag

set s1=skybox.vert
set s2=skybox.frag

set u1=ui.frag
set u2=ui.vert


%glslc% %f1% -o %f1%.spv
%glslc% %f2% -o %f2%.spv
%glslc% %i1% -o %i1%.spv
%glslc% %b1% -o %b1%.spv
%glslc% %b2% -o %b2%.spv

%glslc% %p1% -o %p1%.spv
%glslc% %p2% -o %p2%.spv

%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv

pause