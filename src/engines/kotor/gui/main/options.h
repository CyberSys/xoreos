/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  The options menu.
 */

#ifndef ENGINES_KOTOR_GUI_MAIN_OPTIONS_H
#define ENGINES_KOTOR_GUI_MAIN_OPTIONS_H

#include <memory>

#include "src/graphics/aurora/highlightable.h"

#include "src/engines/kotorbase/gui/gui.h"

#include "src/engines/kotor/gui/options/gameplay.h"

namespace Engines {

namespace KotOR {

class Version;

class OptionsMenu : public KotORBase::GUI {
public:
	OptionsMenu(const Version &gameVersion, ::Engines::Console *console = 0);
	~OptionsMenu();

protected:
	void callbackActive(Widget &widget);

private:
	std::unique_ptr<KotORBase::GUI> _gameplay;
	std::unique_ptr<KotORBase::GUI> _feedback;
	std::unique_ptr<KotORBase::GUI> _autopause;
	std::unique_ptr<KotORBase::GUI> _graphics;
	std::unique_ptr<KotORBase::GUI> _sound;
};

} // End of namespace KotOR

} // End of namespace Engines

#endif // ENGINES_KOTOR_GUI_MAIN_OPTIONS_H
