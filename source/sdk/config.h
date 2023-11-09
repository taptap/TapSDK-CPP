//
// Created by gy939 on 2023/11/9.
//

#pragma once

#ifdef WIN32
#define TDS_EXPORT __declspec(dllexport)
#else
#define TDS_EXPORT
#endif
