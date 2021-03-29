#include "SpeakerAPI.h"
#include "MicrophoneAPI.h"
#include <stdlib.h>
#include <stdio.h>
int main (void)
{
   //Frequency Detection Function
   //Decible Detection Funtion
   //Push Signal to the Speaker API at proper Decible
}

int freqDet(int inFreq)
{
    //Basic PseudoCode

    //Input frequency variable inFreq

    //If in freq > 500 and less than 1500
        //If multiple inputs say maybe 10 if inputs are at 1ms are within 
        //that range hand off to machine learning to determine if indeed 
        //siren.
            //If Siren == true
                //Do nothing
            //Else
                //Signal to the ANC API to phase shift
        //ELse
            //Do nothing
    //Else if in freq > 3000 and less than 4000
        //If multiple inputs say maybe 10 if inputs are at 1ms are within 
        //that range hand off to machine learning to determine if indeed 
        //siren.
            //If Siren == true
                //Do nothing
            //Else
                //Signal to the ANC API to phase shift
        //ELse
            //Do nothing
    //Else
        //Do nothing

}

int decDet(int inDec)
{
    //Basic PsedoCode

    //Input Decible variable inDec

    //return inDec
}