/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "syncrechook.h"

#include "_rtti.h"
#include "abstract.h"
#include "anim.h"
#include "animtype.h"
#include "coord.h"
#include "event.h"
#include "face.h"
#include "globals.h"
#include "mission.h"
#include "object.h"
#include "random.h"
#include "scenario.h"
#include "except.h"
#include "session.h"

#include <mach-o/dyld.h>
#include <mach-o/getsect.h>


static uintptr_t ModuleBase = 0;
static uint32_t ModuleSize = 0;
static uint32_t MapImageBase = 0;


static uint32_t Sync_Caller_RVA(uintptr_t caller)
{
	uintptr_t const address = caller;
	if (ModuleBase != 0 && address >= ModuleBase && address < ModuleBase + ModuleSize) {
		uint32_t const rva = (uint32_t)(address - ModuleBase);
		// The flag bit is only free because the image is far smaller than two gigabytes, so an
		// offset that reached it would be read back as an address outside the image.
		if ((rva & SYNC_CALLER_EXTERN) == 0) {
			return(rva);
		}
	}
	// An address outside the image only says the call came from elsewhere; its low bits are kept
	// to tell such callers apart.
	return(SYNC_CALLER_EXTERN | ((uint32_t)address & ~SYNC_CALLER_EXTERN));
}


// One description per call site rather than per entry: a report holds thousands of entries drawn
// from a few dozen sites, and each lookup takes the symbol handler's lock.
namespace {
	struct CallerTextType {
		uint32_t Rva;
		bool Known;
		char Text[192];
	};

	CallerTextType CallerTexts[256];
	unsigned CallerTextCount = 0;
}


static char const * Sync_Describe_Caller(uint32_t rva)
{
	for (unsigned i = 0; i < CallerTextCount; i++) {
		if (CallerTexts[i].Rva == rva) {
			return(CallerTexts[i].Known ? CallerTexts[i].Text : nullptr);
		}
	}

	if (CallerTextCount >= ARRAY_SIZE(CallerTexts) || ModuleBase == 0) {
		return(nullptr);
	}

	CallerTextType & entry = CallerTexts[CallerTextCount];
	entry.Rva = rva;
	entry.Text[0] = '\0';

	// The recorded address is the instruction after the call, which can belong to the next line
	// or, for a call in tail position, the next function.
	entry.Known = Describe_Code_Address((void const *)(ModuleBase + rva - 1),
					entry.Text, sizeof(entry.Text));

	CallerTextCount++;

	return(entry.Known ? entry.Text : nullptr);
}


void Sync_Record_Random_Impl(Random2Class const & gen, int value, int minval, int maxval, bool ranged, uintptr_t caller)
{
	SyncRandomEntryType entry {};
	entry.Frame = Frame;
	entry.Caller = Sync_Caller_RVA(caller);
	entry.Index1 = gen.Index_1();
	entry.Index2 = gen.Index_2();
	entry.Value = value;
	entry.Min = minval;
	entry.Max = maxval;
	entry.Kind = (Scen != nullptr && &gen == &Scen->RandomNumber) ? SYNC_RANDOM_CRITICAL : SYNC_RANDOM_NONCRITICAL;
	entry.Shape = ranged ? SYNC_DRAW_RANGED : (SyncRecorder.In_Ranged_Draw() ? SYNC_DRAW_INNER : SYNC_DRAW_PLAIN);
	SyncRecorder.Add_Random(entry);
}


void Sync_Record_Facing_Impl(DirType const & facing, uintptr_t caller)
{
	SyncFacingEntryType entry {};
	entry.Frame = Frame;
	entry.Caller = Sync_Caller_RVA(caller);
	entry.Facing = (int16_t)(int)facing.As_Dir256();
	entry.Kind = SYNC_FACING_SET;
	SyncRecorder.Add_Facing(entry);
}


void Sync_Record_Target_Impl(AbstractClass const & subject, AbstractClass const * target, uintptr_t caller)
{
	SyncTargetEntryType entry {};
	entry.Frame = Frame;
	entry.Caller = Sync_Caller_RVA(caller);
	entry.SubjectRTTI = (int8_t)subject.Fetch_RTTI();
	entry.SubjectID = subject.Fetch_ID();
	entry.TargetRTTI = (int8_t)(target != nullptr ? target->Fetch_RTTI() : RTTI_NONE);
	entry.TargetID = (target != nullptr ? target->Fetch_ID() : 0);
	SyncRecorder.Add_Target(entry);
}


void Sync_Record_Mission_Impl(ObjectClass const & subject, int before, int after, int kind, uintptr_t caller)
{
	SyncMissionEntryType entry {};
	entry.Frame = Frame;
	entry.Caller = Sync_Caller_RVA(caller);
	entry.SubjectRTTI = (int8_t)subject.Fetch_RTTI();
	entry.SubjectID = subject.Fetch_ID();
	entry.Before = (int8_t)before;
	entry.After = (int8_t)after;
	entry.Kind = (uint8_t)kind;
	SyncRecorder.Add_Mission(entry);
}


void Sync_Record_Anim_Impl(AnimClass const & anim, Coord const & coord, uintptr_t caller)
{
	SyncAnimEntryType entry {};
	entry.Frame = Frame;
	entry.Caller = Sync_Caller_RVA(caller);
	entry.AnimID = anim.Fetch_ID();
	entry.TypeHeapID = (int16_t)(anim.Class != nullptr ? anim.Class->Fetch_Heap_ID() : -1);
	entry.X = coord.X;
	entry.Y = coord.Y;
	entry.Z = coord.Z;
	SyncRecorder.Add_Anim(entry);
}


void Sync_Record_Event_Impl(EventClass const & event, int source)
{
	switch (event.Type) {
		case EventClass::FRAMEINFO:
		case EventClass::FRAMESYNC:
		case EventClass::RESPONSE_TIME:
		case EventClass::PROCESS_TIME:
		case EventClass::TIMING:
			return;
		default:
			break;
	}

	SyncEventEntryType entry {};
	entry.Frame = event.Frame;
	entry.SeenAt = Frame;
	entry.House = event.ID;
	entry.Type = event.Type;
	entry.Source = (uint8_t)source;

	unsigned length = 0;
	if (event.Type < EventClass::LAST_EVENT) {
		length = EventClass::EventLength[event.Type];
	}
	if (length > sizeof(entry.Bytes)) {
		length = sizeof(entry.Bytes);
	}
	entry.Length = (uint8_t)length;
	memcpy(entry.Bytes, &event.Data, length);

	if (source == SYNC_EVENT_QUEUED) {
		SyncRecorder.Add_Queued_Event(entry);
	} else {
		SyncRecorder.Add_Executed_Event(entry);
	}
}


void Sync_Recorder_Arm(void)
{
	SyncRecorder.Reset();

	bool const network = (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET);
	SyncRecorder.Set_Recording(network || Session.Record || Session.Play);

	// Offsets are taken from the executable's Mach-O header, where __TEXT begins. The linker's
	// preferred base for __TEXT, 0x100000000, does not fit the report's 32-bit field, so
	// MapImageBase stays zero and reports give image offsets alone.
	ModuleBase = (uintptr_t)_dyld_get_image_header(0);
	segment_command_64 const * text = getsegbyname("__TEXT");
	ModuleSize = (text != nullptr && text->vmsize < SYNC_CALLER_EXTERN) ? (uint32_t)text->vmsize : 0;
	MapImageBase = 0;

	SyncCallerContextType context;
	context.MapImageBase = MapImageBase;
	context.Describe = Sync_Describe_Caller;
	Sync_Set_Caller_Context(context);
}


void Sync_Recorder_Disarm(void)
{
	SyncRecorder.Set_Recording(false);
	SyncRecorder.Reset();
}


static char const * Anim_Name(int heap_id)
{
	if (heap_id >= 0 && heap_id < AnimTypes.Count()) {
		return(AnimTypes[heap_id]->Name());
	}
	return("?");
}


static char const * Mission_Name(int mission)
{
	return(MissionClass::Mission_Name((MissionType)mission));
}


static char const * Rtti_Name(int rtti)
{
	return(Name_From_RTTI((RTTIType)rtti));
}


static char const * Event_Name(int type)
{
	if (type >= 0 && type < EventClass::LAST_EVENT) {
		return(EventClass::EventNames[type]);
	}
	return("?");
}


SyncNamesType const & Sync_Engine_Names(void)
{
	static SyncNamesType const names = { Rtti_Name, Mission_Name, Event_Name, Anim_Name };
	return(names);
}
