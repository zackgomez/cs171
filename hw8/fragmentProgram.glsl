uniform float t;
uniform int perFragCalc;
varying vec2 coord;

uniform sampler2D sky, leaf;

const float h = 0.09;
const float a = 7;
const float speed = 2.5;



void main()
{
    //gl_FragColor = vec4(sin(40*u),sin(40*v),0.0,1.0);
    vec4 leafcol = texture2D(leaf, 3*gl_TexCoord[0]);
    vec4 color = texture2D(sky, gl_TexCoord[1]) + leafcol * leafcol.a;

    if (perFragCalc == 1)
    {
        float x = coord.x;
        float y = coord.y;
        float z = h * cos(a * sqrt(x*x + y*y) + speed * t);
        vec3 world = vec3(x, y, z);

        // Calculate vertex normal
        float dx = -a*h* x * sin(a * sqrt(x*x + y*y) + speed * t) / sqrt(x*x + y*y);
        float dy = -a*h* y * sin(a * sqrt(x*x + y*y) + speed * t) / sqrt(x*x + y*y);
        vec3 normal = normalize(vec3(dy, -dx * dy, dx));

        vec3 cam = normalize(vec3(gl_ModelViewMatrix * vec4(world, 1.0)));
        vec3 norm = normalize(gl_NormalMatrix * normal);
        vec3 outVec = reflect(cam, norm);

        float m = 2.0*sqrt(outVec.x * outVec.x
                         + outVec.y * outVec.y
                         + (outVec.z + 1) * (outVec.z + 1));
        vec2 skycoord = vec2(outVec.x / m + 0.5, outVec.y / m + 0.5);

        vec4 leafcol = texture2D(leaf, 3*gl_TexCoord[0]);
        color = texture2D(sky, skycoord) + leafcol * leafcol.a;
    }

    gl_FragColor = color;
}
