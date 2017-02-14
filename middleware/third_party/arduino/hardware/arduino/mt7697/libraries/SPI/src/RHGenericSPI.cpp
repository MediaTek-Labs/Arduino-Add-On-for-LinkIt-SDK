// RHGenericSPI.cpp
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// Contributed by Joanna Rutkowska
// $Id: RHGenericSPI.cpp,v 1.2 2014/04/12 05:26:05 mikem Exp $

#include <RHGenericSPI.h>

RHGenericSPI::RHGenericSPI(RHSPIFrequency frequency, RHSPIBitOrder bitOrder, RHSPIDataMode dataMode)
    :
    _frequency(frequency),
    _bitOrder(bitOrder),
    _dataMode(dataMode)
{
}

void RHGenericSPI::setBitOrder(RHSPIBitOrder bitOrder)
{
    _bitOrder = bitOrder;
}

void RHGenericSPI::setDataMode(RHSPIDataMode dataMode)
{
    _dataMode = dataMode;
}

void RHGenericSPI::setFrequency(RHSPIFrequency frequency)
{
    _frequency = frequency;
}

