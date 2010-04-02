/* eos - A reimplementation of BioWare's Aurora engine
 * Copyright (c) 2010 Sven Hesse (DrMcCoy), Matthew Hoops (clone2727)
 *
 * The Infinity, Aurora, Odyssey and Eclipse engines, Copyright (c) BioWare corp.
 * The Electron engine, Copyright (c) Obsidian Entertainment and BioWare corp.
 *
 * This file is part of eos and is distributed under the terms of
 * the GNU General Public Licence. See COPYING for more informations.
 */

/** @file aurora/gffstructs.h
 *  %Common structures found in BioWare's GFF files.
 */

#ifndef AURORA_GFFSTRUCTS_H
#define AURORA_GFFSTRUCTS_H

#include "common/types.h"

#include "aurora/types.h"
#include "aurora/gfffile.h"

namespace Aurora {

/** A location in a module. */
class GFFLocation {
public:
	GFFLocation(      uint32  area        = kObjectIDInvalid,
	            const double *position    = 0,
	            const double *orientation = 0);

	void clear(); ///< Clear the location.

	      uint32  getArea       () const; ///< Return the ObjectID of the area.
	const double *getPosition   () const; ///< Return the coordinates of the location.
	const double *getOrientation() const; ///< Return the direction vector.

	void setArea       (uint32 area);               ///< Set the ObjectID of the area.
	void setPosition   (const double *position);    ///< Set the coordinates of the location.
	void setOrientation(const double *orientation); ///< Set the direction vector.

	/** Read the location out of a GFF struct. */
	bool read(const GFFFile::StructRange &range);

private:
	uint32 _area;           ///< ObjectID of the area containing the location.
	double _position[3];    ///< (x,y,z) coordinates of the location.
	double _orientation[3]; ///< (x,y,z) components of the direction vector the location faces.

	void clearPosition();
	void clearOrientation();
};

/** A scripting variable and its value. */
class GFFVariable {
};

/** A list of scripting variables and their values. */
class GFFVarTable {
};

/** An effect on an object. */
class GFFEffect {
};

/** A list of effects on an object. */
class GFFEffectsList {
};

/** A game event. */
class GFFEvent {
};

/** A queue of game events. */
class GFFEventQueue {
};

/** An action on objects. */
class GFFAction {
};

/** A list of actions on objects. */
class GFFActionList {
};

} // End of namespace Aurora

#endif // AURORA_GFFSTRUCTS_H
