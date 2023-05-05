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

set caster1=shadowCaster.vert
set caster2=shadowCaster.frag
set caster3=shadowCasterInstancing.vert

set geo1=geo_debug.vert
set geo2=geo_debug.frag
set geo3=geo_debug.geom
set geo4=geo_debug_instancing.vert
set geo5=geo_debug_instancing.geom

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


%glslc% %caster1% -o %caster1%.spv
%glslc% %caster2% -o %caster2%.spv
%glslc% %caster3% -o %caster3%.spv

%glslc% %geo1% -o %geo1%.spv
%glslc% %geo2% -o %geo2%.spv
%glslc% %geo3% -o %geo3%.spv
%glslc% %geo4% -o %geo4%.spv
%glslc% %geo5% -o %geo5%.spv
pause