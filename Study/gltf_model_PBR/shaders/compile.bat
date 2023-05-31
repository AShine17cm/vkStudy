set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag
set p1=pbr.vert
set p2=pbr.frag
set pt1=point.vert
set pt2=point.frag

set caster1=shadowCaster.vert
set caster2=shadowCaster.geom
set caster3=shadowCaster_gltf.vert

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv
%glslc% %p1% -o %p1%.spv
%glslc% %p2% -o %p2%.spv
%glslc% %pt1% -o %pt1%.spv
%glslc% %pt2% -o %pt2%.spv

%glslc% %caster1% -o %caster1%.spv
%glslc% %caster2% -o %caster2%.spv
%glslc% %caster3% -o %caster3%.spv

pause