set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag
set s1=tex.vert
set s2=tex.frag
set a2=texArray.frag
set c2=texCube.frag

set caster1=shadowCaster.vert
set caster2=shadowCaster.geom
set caster3=shadowCaster_gltf.vert

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv
%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv
%glslc% %a2% -o %a2%.spv
%glslc% %c2% -o %c2%.spv

%glslc% %caster1% -o %caster1%.spv
%glslc% %caster2% -o %caster2%.spv
%glslc% %caster3% -o %caster3%.spv

pause