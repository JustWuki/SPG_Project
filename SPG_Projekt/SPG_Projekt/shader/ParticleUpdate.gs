#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 40) out;

// All that we get from vertex shader

in vec3 vPositionPass[];
in vec3 vVelocityPass[];
in vec3 vColorPass[];
in float vLifeTimePass[];
in float vSizePass[];
in int vTypePass[];

// All that we send further

out vec3 vPositionOut;
out vec3 vVelocityOut;
out vec3 vColorOut;
out float vLifeTimeOut;
out float vSizeOut;
out int vTypeOut;

uniform vec3 uGenPosition; // Position where new particles are spawned
uniform vec3 uGenGravityVector; // Gravity vector for particles - updates velocity of particles 
uniform vec3 uGenVelocityMin; // Velocity of new particle - from min to (min+range)
uniform vec3 uGenVelocityRange;

uniform vec3 uGenColor;
uniform float uGenSize; 

uniform float uGenLifeMin, uGenLifeRange; // Life of new particle - from min to (min+range)
uniform float uTimePassed; // Time passed since last frame

uniform vec3 uRandomSeed; // Seed number for our random number function
vec3 localSeed;

uniform int uNumToGenerate; // How many particles will be generated next time, if greater than zero, particles are generated

// This function returns random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(localSeed.y * 214013.0 + localSeed.x * 2531011.0 + localSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    localSeed = vec3(localSeed.x + 147158.0 * fRes, localSeed.y*fRes  + 415161.0 * fRes, localSeed.z + 324154.0*fRes);
    return fRes;
}

void main()
{
  localSeed = uRandomSeed;
  
  // gl_Position doesn't matter now, as rendering is discarded, so I don't set it at all

  vPositionOut = vPositionPass[0];
  vVelocityOut = vVelocityPass[0];
  if(vTypePass[0] != 0)
  {
    vPositionOut += vVelocityOut * uTimePassed;
  }
  if(vTypePass[0] != 0)
  {
    vVelocityOut += uGenGravityVector * uTimePassed;
  }

  vColorOut = vColorPass[0];
  vLifeTimeOut = vLifeTimePass[0] - uTimePassed;
  vSizeOut = vSizePass[0];
  vTypeOut = vTypePass[0];
    
  if(vTypeOut == 0)
  {
    EmitVertex();
    EndPrimitive();
    
    for(int i = 0; i < uNumToGenerate; i++)
    {
      vPositionOut = uGenPosition;
      vVelocityOut = uGenVelocityMin + vec3(uGenVelocityRange.x*randZeroOne(), uGenVelocityRange.y*randZeroOne(), uGenVelocityRange.z*randZeroOne());
      vColorOut = uGenColor;
      vLifeTimeOut = uGenLifeMin + uGenLifeRange * randZeroOne();
      vSizeOut = uGenSize;
      vTypeOut = 1;
      EmitVertex();
      EndPrimitive();
    }
  }
  else if(vLifeTimeOut > 0.0)
  {
      EmitVertex();
      EndPrimitive(); 
  }
}