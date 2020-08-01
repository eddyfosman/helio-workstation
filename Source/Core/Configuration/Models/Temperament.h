/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "BaseResource.h"
#include "Note.h"

class Temperament final : public BaseResource
{
public:

    Temperament() = default;
    Temperament(Temperament &&other) noexcept;
    Temperament(const Temperament &other) noexcept;

    String getResourceId() const noexcept override;
    Identifier getResourceType() const noexcept override;

    using Period = StringArray;
    using Ptr = ReferenceCountedObjectPtr<Temperament>;
    
    inline auto getPeriodSize() const noexcept
    {
        return this->period.size();
    }

    inline auto getNumKeys() const noexcept
    {
        return this->keysTotal;
    }

    inline auto getMiddleC() const noexcept
    {
        return this->middleC;
    }

    String getMidiNoteName(Note::Key note, bool includePeriod) const noexcept;

    //===------------------------------------------------------------------===//
    // Hard-coded defaults
    //===------------------------------------------------------------------===//

    static Temperament::Ptr getTwelveToneEqualTemperament();

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//

    SerializedData serialize() const override;
    void deserialize(const SerializedData &data) override;
    void reset() override;

    //===------------------------------------------------------------------===//
    // Operators
    //===------------------------------------------------------------------===//

    Temperament &operator=(const Temperament &other);
    friend bool operator==(const Temperament &l, const Temperament &r);
    friend bool operator!=(const Temperament &l, const Temperament &r);
    
    int hashCode() const noexcept;

private:

    String id;
    String name;
    Period period;

    Note::Key middleC = 0;
    int keysTotal = 0;

    static constexpr auto periodNumForMiddleC = 5;
    static constexpr auto displayedPeriodNumForMiddleC = 3;

    JUCE_LEAK_DETECTOR(Temperament)
};