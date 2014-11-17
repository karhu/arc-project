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
            {mat4, "transform.view_proj"},  -- view and projection matrix
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

        out.projected = transform.view_proj * transform.model * vec4(in.position,1);
        out.normal = vec3(transform.view_proj * transform.model * vec4(in.normal,0));
        out.color = instance.color;
        
    ]],
}

fragment = {
    main = [[
    	vec3 normal = normalize(in.normal);

        vec3 to_light = normalize(vec3(1,1,0));
        float intensity = max(0,dot(to_light,normal));

        vec3 ambient = vec3(1,1,1);

        // (0.05*ambient + 0.7*intensity)
        vec3 color =  0.75*in.color;

        float gamma = 2.2;
        color = pow(color, vec3(1.0 / gamma));

        gl_FragColor = vec4(color,1);
    ]],
}
