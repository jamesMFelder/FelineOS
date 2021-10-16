// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder

//If assembly or undefined behavior creates a call to a pure virtual function
//	Given that this never should be called, we can't assume anything
//	so just don't do anything
extern "C" void __cxa_pure_virtual(){}
