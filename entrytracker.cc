/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string>
#include <sstream>

#include "util.h"

#include "entrytracker.h"

EntryTracker::EntryTracker() {
    next_field_num = 1;
}

int EntryTracker::RegisterField(string in_name, TrackerType in_type,
                                string in_desc, string in_location) {
    string mod_name = StrLower(in_name);

    if (field_name_map.find(mod_name) != field_name_map.end()) {
        fprintf(stderr, "DEBUG %s:%d - Tried to register %s when already registered, from %s\n", __FILE__, __LINE__, mod_name.c_str(), in_location.c_str());
        return -1;
    }

    reserved_field *definition = new reserved_field();

    definition->field_id = next_field_num++;
    definition->field_name = in_name;

    definition->track_type = in_type;
    definition->builder = NULL;

    definition->field_description = in_desc;
    definition->field_location = in_location;

    field_name_map[mod_name] = definition;
    field_id_map[definition->field_id] = definition;

    return definition->field_id;
}

int EntryTracker::RegisterField(string in_name, TrackerElement *in_builder,
                                string in_desc, string in_location) {
    string mod_name = StrLower(in_name);

    if (field_name_map.find(mod_name) != field_name_map.end()) {
        fprintf(stderr, "DEBUG %s:%d - Tried to register %s when already registered, from %s\n", __FILE__, __LINE__, mod_name.c_str(), in_location.c_str());
        return -1;
    }

    reserved_field *definition = new reserved_field();

    definition->field_id = next_field_num++;
    definition->field_name = in_name;

    definition->builder = in_builder;

    definition->field_description = in_desc;
    definition->field_location = in_location;

    field_name_map[mod_name] = definition;
    field_id_map[definition->field_id] = definition;

    return definition->field_id;
}

int EntryTracker::GetFieldId(string in_name) {
    string mod_name = StrLower(in_name);

    map<string, reserved_field *>::iterator itr = field_name_map.find(mod_name);

    if (itr == field_name_map.end()) {
        return -1;
    }

    return itr->second->field_id;
}

string EntryTracker::GenerateLocationString(const char *in_file, const int in_line, 
                                            const char *in_func) {
    std::stringstream ss;
    ss << in_file << ":" << in_line << " " << in_func;
    return ss.str();
}

TrackerElement *EntryTracker::RegisterAndGetField(string in_name, TrackerType in_type,
                                                  string in_desc, string in_location) {
    int fn = GetFieldId(in_name);

    if (fn >= 0) {
        return GetTrackedInstance(fn);
    }

    fn = RegisterField(in_name, in_type, in_desc, in_location);

    return new TrackerElement(in_type, fn);
}

TrackerElement *EntryTracker::RegisterAndGetField(string in_name, TrackerElement *in_builder,
                                                  string in_desc, string in_location) {
    int fn = GetFieldId(in_name);

    if (fn >= 0) {
        return GetTrackedInstance(fn);
    }

    fn = RegisterField(in_name, in_builder, in_desc, in_location);

    TrackerElement *clone = in_builder->clone(fn);

    return clone;
}


TrackerElement *EntryTracker::GetTrackedInstance(int in_id) {
    map<int, reserved_field *>::iterator itr = field_id_map.find(in_id);

    if (itr == field_id_map.end()) {
        return NULL;
    }

    reserved_field *definition;
    TrackerElement *ret;

    definition = itr->second;

    if (definition->builder == NULL)
        ret = new TrackerElement(definition->track_type, definition->field_id);
    else
        ret = definition->builder->clone(definition->field_id);

    return ret;

}

TrackerElement *EntryTracker::GetTrackedInstance(string in_name) {
    string mod_name = StrLower(in_name);

    map<string, reserved_field *>::iterator itr = field_name_map.find(mod_name);

    reserved_field *definition;
    TrackerElement *ret;

    // We don't know this
    if (itr == field_name_map.end()) {
        return NULL;
    }

    definition = itr->second;

    if (definition->builder == NULL)
        ret = new TrackerElement(definition->track_type, definition->field_id);
    else
        ret = definition->builder->clone(definition->field_id);

    return ret;
}

