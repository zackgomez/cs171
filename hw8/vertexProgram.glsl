uniform float t;
uniform int perFragCalc;

const float h = 0.09;
const float a = 7;
const float speed = 2.5;
varying vec2 coord;

void main()
{
   // Convert from the [-5,5]x[-5,5] range provided into radians
   // between 0 and 2*theta
   float u = (gl_Vertex.x + 5.0) / 10.0;
   float v = (gl_Vertex.y + 5.0) / 10.0;


   // Calculate vertex position
   float x = gl_Vertex.x;
   float y = gl_Vertex.y;
   float z = h * cos(a * sqrt(x*x + y*y) + speed * t);
   vec3 world = vec3(x, y, z);

   // Calculate vertex normal
   float dx = -a*h* x * sin(a * sqrt(x*x + y*y) + speed * t) / sqrt(x*x + y*y);
   float dy = -a*h* y * sin(a * sqrt(x*x + y*y) + speed * t) / sqrt(x*x + y*y);
   vec3 normal = normalize(vec3(dy, -dx * dy, dx));

   // Calculate camera vector
   vec3 cam = normalize(vec3(gl_ModelViewMatrix * vec4(world, 1.0)));
   vec3 norm = normalize(gl_NormalMatrix * normal);
   vec3 outVec = reflect(cam, norm);

   float m = 2.0*sqrt(outVec.x * outVec.x + outVec.y * outVec.y + (outVec.z + 1) * (outVec.z + 1));

   // Put the leaf coords in gl_TexCoord[0]
   gl_TexCoord[0].x = u;
   gl_TexCoord[0].y = v;
   // put the sky coords in gl_TexCoord[1]
   gl_TexCoord[1].x = outVec.x / m + 0.5;
   gl_TexCoord[1].y = outVec.y / m + 0.5;
   coord = gl_Vertex.xy;


   gl_Position = gl_ModelViewProjectionMatrix * vec4(world,1.0);
}
