//  Cartoon fragment shader
//  Adapted from Lighthouse3D

varying vec3 N;
varying vec3 L;
	
void main()
{
   float intensity = dot(normalize(N),normalize(L));

   float f;
   if (intensity > 0.95)
      f= 1.0;
   else if (intensity > 0.5)
      f = 0.6;
   else if (intensity > 0.25)
      f = 0.4;
   else
      f = 0.2;

   gl_FragColor = vec4(1.0*f,0.5*f,0.5*f,1.0);
}
