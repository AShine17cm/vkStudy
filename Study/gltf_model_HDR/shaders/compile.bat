set glslc=C:/VulkanSDK/1.3.216.0/Bin/glslc.exe
set u1=ui.vert
set u2=ui.frag
set p1=pbr_basic.vert
set p2=pbr_basic.frag
set p3=pbr_ibl.frag

set pt1=point.vert
set pt2=point.frag

set caster1=shadowCaster.vert
set caster2=shadowCaster.geom
set caster3=shadowCaster_gltf.vert

set scr=screen.vert
set hdr1=hdr_offscreen.frag
set hdr2=hdr_bloom.frag
set ldr=ldr.frag
set bl=blend.frag

%glslc% %u1% -o %u1%.spv
%glslc% %u2% -o %u2%.spv
%glslc% %p1% -o %p1%.spv
%glslc% %p2% -o %p2%.spv
%glslc% %p3% -o %p3%.spv

%glslc% %pt1% -o %pt1%.spv
%glslc% %pt2% -o %pt2%.spv

%glslc% %caster1% -o %caster1%.spv
%glslc% %caster2% -o %caster2%.spv
%glslc% %caster3% -o %caster3%.spv

%glslc% %scr% -o %scr%.spv
%glslc% %hdr1% -o %hdr1%.spv
%glslc% %hdr2% -o %hdr2%.spv
%glslc% %ldr% -o %ldr%.spv
%glslc% %bl% -o %bl%.spv

pause