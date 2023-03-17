#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>

#include <sstream>

#include "collision_detector.h"

namespace Catch {
template<>
struct StringMaker<collision_detector::GatheringEvent> {
    static std::string convert(collision_detector::GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(" << value.gatherer_id << value.item_id << value.sq_distance << value.time << ")";

        return tmp.str();
    }
};
}  // namespace Catch

namespace {

using Catch::Matchers::IsEmpty;

template <typename Range, typename Predicate>
struct EqualsRangeMatcher : Catch::Matchers::MatcherGenericBase {
    EqualsRangeMatcher(Range const& range, Predicate predicate)
        : range_{range}
        , predicate_{predicate} {
    }

    template <typename OtherRange>
    bool match(const OtherRange& other) const {
        using std::begin;
        using std::end;

        return std::equal(begin(range_), end(range_), begin(other), end(other), predicate_);
    }

    std::string describe() const override {
        return "Equals: " + Catch::rangeToString(range_);
    }

private:
    const Range& range_;
    Predicate predicate_;
};

template <typename Range, typename Predicate>
auto EqualsRange(const Range& range, Predicate prediate) {
    return EqualsRangeMatcher<Range, Predicate>{range, prediate};
}
class ItemGathererProviderMock : public collision_detector::ItemGathererProvider {
public:
    ItemGathererProviderMock(std::vector<collision_detector::Item> items,
                             std::vector<collision_detector::Gatherer> gatherers)
        : items_(items)
        , gatherers_(gatherers) {
    }

    
    size_t ItemsCount() const override {
        return items_.size();
    }
    collision_detector::Item GetItem(size_t idx) const override {
        return items_[idx];
    }
    size_t GatherersCount() const override {
        return gatherers_.size();
    }
    collision_detector::Gatherer GetGatherer(size_t idx) const override {
        return gatherers_[idx];
    }

private:
    std::vector<collision_detector::Item> items_;
    std::vector<collision_detector::Gatherer> gatherers_;
};

class CompareEvents {
public:
    bool operator()(const collision_detector::GatheringEvent& l,
                    const collision_detector::GatheringEvent& r) {
        if (l.gatherer_id != r.gatherer_id || l.item_id != r.item_id) 
            return false;

        static const double eps = 1e-10;

        if (std::abs(l.sq_distance - r.sq_distance) > eps) {
            return false;
        }

        if (std::abs(l.time - r.time) > eps) {
            return false;
        }
        return true;
    }
};

} 

SCENARIO("Collision detector") {
    WHEN("no items") {
        ItemGathererProviderMock provider{
            {}, {{{5, 0}, {-5, 0}, 5.0}, {{0, -5}, {0, 5}, 5.0}, {{-5, -5}, {5, 5}, 5.0}, {{-5, 5}, {5, -5}, 5.0}}};
        THEN("No events") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.empty());
        }
    }
    WHEN("no gatherers") {
        ItemGathererProviderMock provider{
            {{{1, 2}, 5.}, {{0, 0}, 5.}, {{-5, 0}, 5.}}, {}};
        THEN("No events") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.empty());
        }
    }
    WHEN("multiple items on a way of gatherer") {
        ItemGathererProviderMock provider{{
            // items
            {{9, 2}, 0.5},
            {{8, 1.8}, 0.5},
            {{7, 1.5}, 0.5},
            {{6, 1.2}, 0.5},
            {{5, 1.01}, 0.5},
            {{4, 1.0}, 0.5},
            {{3, 0.99}, 0.5},
            {{2, 0.7}, 0.5},
            {{1, 0.4}, 0.5},
            {{0, 0.0}, 0.5},
            {{-2, 0}, 0.5},
            }, {
            // gatherers
            {{0, 0}, {10, 0}, 0.5},
        }}; 
        THEN("Gathered items in right order") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK_THAT(events, !IsEmpty());
            CHECK_THAT(
                events,
                EqualsRange(std::vector{
                    collision_detector::GatheringEvent{9, 0,0.*0., 0.0},
                    collision_detector::GatheringEvent{8, 0,0.4*0.4, 0.1},
                    collision_detector::GatheringEvent{7, 0,0.7*0.7, 0.2},
                    collision_detector::GatheringEvent{6, 0,0.99*0.99, 0.3},
                    collision_detector::GatheringEvent{5, 0,1*1, 0.4},
                }, CompareEvents()));
        }
    }
    WHEN("multiple gatherers on one item") {
        ItemGathererProviderMock provider{{
            // items
            {{0, 0}, 0.1},
            }, {
            // gatherers
            {{-1, 0}, {1, 0}, 0.5}, // ~ 0.5 * tick_period
            {{-10, -10}, {10, 10}, 0.5}, // ~ 0.5 * tick_period
            {{-100, 10}, {10, -1}, 0.5}, // ~ 0.9 * tick_period
            {{0, -10}, {0, 100}, 0.5}, // <-- fastest ~ 0.1 * tick_period

        }};
        THEN("Fastest takes an item") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK_THAT(events, !IsEmpty());
            CHECK(events.front().gatherer_id == 3); 
        }
    }
    WHEN("No motion") {
        ItemGathererProviderMock provider{{
            // items
            {{0, 0}, 0.1},
            }, {
            // gatherers
            {{-1, 0}, {-1, 0}, 0.5}, 
            {{-10, -10}, {-10, -10}, 0.5}, 
            {{-100, 10}, {-100, 10}, 0.5}, 
            {{0, 10}, {0, 10}, 0.5}, 
        }};
        THEN("No events") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.empty());
        }
    }
}

// Напишите здесь тесты для функции collision_detector::FindGatherEvents