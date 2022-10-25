#ifndef FORMATS_H
#define FORMATS_H

const uint R11G11B10F = 0;
const uint R10G10B10A2F = R11G11B10F + 1;
const uint R16F = R10G10B10A2F + 1;
const uint R16G16F = R16F + 1;
const uint R16G16B16A16F = R16G16F + 1;
const uint R8 = R16G16B16A16F + 1;
const uint R8G8 = R8 + 1;
const uint R8G8B8A8 = R8G8 + 1;
const uint R16 = R8G8B8A8 + 1;
const uint R16G16 = R16 + 1;
const uint R16G16B16A16 = R16G16 + 1;
const uint R32F = R16G16B16A16 + 1;
const uint R32G32F = R32F + 1;
const uint R32G32B32A32F = R32G32F + 1;
const uint E5B9G9R9F = R32G32B32A32F + 1;

#endif