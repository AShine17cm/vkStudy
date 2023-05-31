set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag
set s1=tex.vert
set s2=tex.frag
set p1=pbr.vert
set p2=pbr.frag
set t1=particle.vert
set t2=particle.frag
set pt1=point.vert
set pt2=point.frag

set caster1=shadowCaster.vert
set caster2=shadowCaster.geom
set caster3=shadowCaster_gltf.vert

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv
%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv
%glslc% %p1% -o %p1%.spv
%glslc% %p2% -o %p2%.spv
%glslc% %t1% -o %t1%.spv
%glslc% %t2% -o %t2%.spv
%glslc% %pt1% -o %pt1%.spv
%glslc% %pt2% -o %pt2%.spv

%glslc% %caster1% -o %caster1%.spv
%glslc% %caster2% -o %caster2%.spv
%glslc% %caster3% -o %caster3%.spv

pause