//
//  Copyright (c) 2017-2019   Finnbarr P. Murphy.   All rights reserved.
//
//  Show PCI devices
//
//  License: UDK2015 license applies to code from UDK2015 source,
//           BSD 2 clause license applies to all other code.
//

// Edited in Aug21 2022 By Pouria Arefi


#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/PciEnumerationComplete.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci.h>
 
#define CALC_EFI_PCI_ADDRESS(Bus, Dev, Func, Reg) \
    ((UINT64) ((((UINTN) Bus) << 24) + (((UINTN) Dev) << 16) + (((UINTN) Func) << 8) + ((UINTN) Reg)))


#pragma pack(1)
typedef union {
   PCI_DEVICE_HEADER_TYPE_REGION  Device;
   PCI_CARDBUS_CONTROL_REGISTER   CardBus;
} NON_COMMON_UNION;

typedef struct {
   PCI_DEVICE_INDEPENDENT_REGION  Common;
   NON_COMMON_UNION               NonCommon;
   UINT32                         Data[48];
} PCI_CONFIG_SPACE;
#pragma pack()

#define UTILITY_VERSION L"20190329"
#undef DEBUG


//
// Copyed from UDK2015 Source. UDK2015 license applies.
//
EFI_STATUS
PciGetNextBusRange( EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  **Descriptors,
                    UINT16 *MinBus,
                    UINT16 *MaxBus,
                    BOOLEAN *IsEnd )
{
    *IsEnd = FALSE;

    if ((*Descriptors) == NULL) {
        *MinBus = 0;
        *MaxBus = PCI_MAX_BUS;
        return EFI_SUCCESS;
    }

    while ((*Descriptors)->Desc != ACPI_END_TAG_DESCRIPTOR) {
        if ((*Descriptors)->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS) {
            *MinBus = (UINT16) (*Descriptors)->AddrRangeMin;
            *MaxBus = (UINT16) (*Descriptors)->AddrRangeMax;
            (*Descriptors)++;
            return (EFI_SUCCESS);
        }

        (*Descriptors)++;
    }

    if ((*Descriptors)->Desc == ACPI_END_TAG_DESCRIPTOR) {
        *IsEnd = TRUE;
    }

    return EFI_SUCCESS;
}


//
// Copyed from UDK2015 Source. UDK2015 license applies.
//
EFI_STATUS
PciGetProtocolAndResource( EFI_HANDLE Handle,
                           EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL **IoDev,
                           EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR **Descriptors )
{
    EFI_STATUS Status;

    // Get inferface from protocol
    Status = gBS->HandleProtocol( Handle,
                                  &gEfiPciRootBridgeIoProtocolGuid,
                                  (VOID**)IoDev );
    if (EFI_ERROR (Status)) {
        return Status;
    }

    Status = (*IoDev)->Configuration( *IoDev, (VOID**)Descriptors );
    if (Status == EFI_UNSUPPORTED) {
        *Descriptors = NULL;
        return EFI_SUCCESS;
    }

    return Status;
}


INTN
EFIAPI
ShellAppMain( UINTN Argc, 
              CHAR16 **Argv )
{
    EFI_GUID gEfiPciEnumerationCompleteProtocolGuid = EFI_PCI_ENUMERATION_COMPLETE_GUID;  
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *IoDev;
    EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptors;
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_HANDLE *HandleBuf;
    UINTN HandleBufSize;
    UINTN HandleCount;
    UINT64 Address;
    UINT16 MinBus;
    UINT16 MaxBus;
    BOOLEAN IsEnd; 
    VOID *Interface;

    // checking if we could find any pci devices
    Status = gBS->LocateProtocol( &gEfiPciEnumerationCompleteProtocolGuid,
                                  NULL,
                                  &Interface );
    // checking if error
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Could not find PCI enumeration protocol\n");
        return Status;
    }

    // initializing array HandleBuf to store handles caught from database and store them
    HandleBufSize = sizeof(EFI_HANDLE);
    HandleBuf = (EFI_HANDLE *) AllocateZeroPool( HandleBufSize);
    if (HandleBuf == NULL) {
        Print(L"ERROR: Out of memory resources\n");
        return EFI_OUT_OF_RESOURCES;
    }

    // allocate some space for the number of handles found on server
    Status = gBS->LocateHandle( ByProtocol,
                            &gEfiPciRootBridgeIoProtocolGuid,
                            NULL,
                            &HandleBufSize,
                            HandleBuf);

    // if we get too small error jus catch it
    if (Status == EFI_BUFFER_TOO_SMALL) {
        HandleBuf = ReallocatePool (sizeof (EFI_HANDLE), HandleBufSize, HandleBuf);
        if (HandleBuf == NULL) {
            Print(L"ERROR: Out of memory resources\n");
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }

        Status = gBS->LocateHandle( ByProtocol,
                                    &gEfiPciRootBridgeIoProtocolGuid,
                                    NULL,
                                    &HandleBufSize,
                                    HandleBuf);
    }

    if (EFI_ERROR (Status)) {
        Print(L"ERROR: Failed to find any PCI handles\n");
        goto Done;
    }

    // get how many handle we caught from database
    HandleCount = HandleBufSize / sizeof (EFI_HANDLE);

    // print the handle size
    Print(L"Halndle count size:%d\n",HandleCount);

    // for any handle we have loop through it
    for (UINT16 Index = 0; Index < HandleCount; Index++) {

        // getting descriptor and PCI IO (EFI_PCI_IO_PROTOCOL) named IoDev from each handler caught from DB
        Status = PciGetProtocolAndResource( HandleBuf[Index],
                                            &IoDev,
                                            &Descriptors);

        // checking possible errors
        if (EFI_ERROR(Status)) {
            Print(L"ERROR: PciGetProtocolAndResource [%d]\n, Status");
            goto Done;
        }

        // printing the address range from min to max
        Print(L"Here tou can find address ranges\r Max:%d Min:%d\t \n",Descriptors->AddrRangeMax,Descriptors->AddrRangeMin);

        // until we have null descriptor or end or any error we try to read and write data onto each pci devices
        while(TRUE) {

            // getting min max address value
            Status = PciGetNextBusRange( &Descriptors, &MinBus, &MaxBus, &IsEnd);

            if (EFI_ERROR(Status)) {
                Print(L"ERROR: Retrieving PCI bus range [%d]\n", Status);
                goto Done;
            }

            // if reading address ended break the loop
            if (IsEnd) {
                break;
            }

            Print(L"\n");

            //Just for Sample using

            // MinBus <=  Bus  <=  MaxBus
            UINT16 Bus = MinBus;
            // 0 <= Device <= PCI_MAX_DEVICE
            UINT16 Device = PCI_MAX_DEVICE;
            // 0 <= Func <= PCI_MAX_FUNC
            UINT16 Func = PCI_MAX_FUNC;
            // Now get physical available address of a PCI
            Address = CALC_EFI_PCI_ADDRESS (Bus, Device, Func, 0);

            Print(L"  Writing to memory started with dummy variable DmaStartAddress \n");

            UINT32 DmaStartAddress = 25;

            // Reading from that PCI

            Status = IoDev->Pci.Read( IoDev,
                                      EfiPciWidthUint8,
                                      Address,
                                      sizeof(DmaStartAddress),
                                      &DmaStartAddress);

            // see what have been written
            Print(L"  DmaStartAddress:%d\n",DmaStartAddress);

            // just trying to write a 1 on that address (Needs more work)
             Status = IoDev->Pci.Write( IoDev,
                                       EfiPciWidthUint8,
                                       1,
                                       sizeof(DmaStartAddress),
                                       &DmaStartAddress);

            // checking possible error
            if(EFI_ERROR(Status)){
                return Status;
            }

            // changing DmaStartAddress value to see if we succeed to write on pci
            Print(L" Change value of DmaStartAddress\n");
            DmaStartAddress = 28;
            Print(L"  DmaStartAddress:%d\n",DmaStartAddress);

            // read that address again
            Status = IoDev->Pci.Read( IoDev,
                                   EfiPciWidthUint8,
                                      Address,
                                   sizeof(DmaStartAddress),
                                   &DmaStartAddress);

            // checking possible error
            if(EFI_ERROR(Status)){
                return Status;
            }

            // print last red value
            Print(L"\n");
            Print(L"  Reading from memory\n");
            Print(L"  DmaStartAddress:%d\n",DmaStartAddress);

            // if we don't see any descriptor break the loop
            if (Descriptors == NULL) {
                break;
            }
        }
    }
    // empty the input
    Print(L"\n");

Done:
    if (HandleBuf != NULL) {
        FreePool( HandleBuf );
    }

    return Status;
}
