version = "440 core"
language = "glsl"


-- per instance data vs per frame data ?
-- think about use case: animation

-- batch settings --

maximum_batch_size = 256

debug_vertex_pre_main = [[
    #extension GL_ARB_shader_draw_parameters : require
]]


vertex = {
    uniforms = {
        instance = {
            {mat4, "transform.model"},      -- model matrix
            {vec3, "instance.color"},       -- object color
        }
    },
    input = {
        {vec3,  "position"},        -- vertex position
        {vec3,  "normal"},          -- vertex normal
    },
    output = {
        {vec3, "color"},            -- vertex color
        {vec3, "normal"},           -- vertex normal
    },
    main = [[
        int _id = gl_InstanceID + gl_BaseInstanceARB;

        int col = _id / 10;
        int row = _id - 10*col;

        vec2 pos2d = 0.18*in.position.xy + vec2(row*0.2,col*0.2) + vec2(-0.9,-0.9);

        out.projected = vec4(pos2d,0,1);
        out.color = instance.color;
    ]],
}

fragment = {
    main = [[
        vec3 col = in.color;
        gl_FragColor = vec4(col,1);
    ]],
}
