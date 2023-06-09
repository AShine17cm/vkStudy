set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag

set s1=geo_mrt.vert
set s2=geo_mrt_tex.frag
set a2=geo_mrt_texArray.frag
set c2=geo_mrt_cubeMap.frag

set df1=deferred_compose.vert
set df2=deferred_compose.frag

set caster1=shadowCaster.vert
set caster2=shadowCaster.geom

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv
%glslc% %s1% -o %s1%.spv
%glslc% %s2% -o %s2%.spv
%glslc% %a2% -o %a2%.spv
%glslc% %c2% -o %c2%.spv

%glslc% %df1% -o %df1%.spv
%glslc% %df2% -o %df2%.spv

%glslc% %caster1% -o %caster1%.spv
%glslc% %caster2% -o %caster2%.spv

pause