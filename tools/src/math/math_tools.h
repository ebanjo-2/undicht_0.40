#ifndef math_tools
#define math_tools

#include <algorithm>
#include <array>


namespace undicht {

    namespace tools {

        template<typename number>
        bool overlappingRanges(number range_start_1, number range_end_1, number range_start_2, number range_end_2) {
            /** "touching" ranges dont overlap */

            // making sure the start of the range is before the end
            number s1 = std::min(range_start_1, range_end_1);
            number e1 = std::max(range_start_1, range_end_1);

            number s2 = std::min(range_start_2, range_end_2);
            number e2 = std::max(range_start_2, range_end_2);

            // cases in which the ranges do not overlap (is it really that easy ?)
            if((e1 <= s2) || (e2 <= s1)) {

                return false;
            }

            return true;
        }



        template<typename number>
        std::array<number,2> getMidValues(number n0, number n1, number n2, number n3) {
            /** @return not the biggest and not the smallest number */

            // there may be a faster way
            std::array<number, 4> sorted = {n0, n1, n2, n3};
            std::sort(sorted.begin(), sorted.end());

            return {sorted[1], sorted[2]};
        }

    }

}


#endif // math_tools
