#include "collision_detector.h"

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
    // Проверим, что перемещение ненулевое.
    // Тут приходится использовать строгое равенство, а не приближённое,
    // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
    // расстояние.
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

// В задании на разработку тестов реализовывать следующую функцию не нужно -
// она будет линковаться извне.

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
    if (provider.ItemsCount() == 0 || provider.GatherersCount() == 0) {
        return {};
    }
    std::vector<GatheringEvent> gathering_events;
    for (size_t gatherer_idx = 0; gatherer_idx < provider.GatherersCount(); ++gatherer_idx) {
        auto gatherer = provider.GetGatherer(gatherer_idx);
        if (gatherer.start_pos == gatherer.end_pos) {
           continue; // if gatherer not moving he cant gather
        }

        for (size_t item_idx = 0; item_idx < provider.ItemsCount(); ++item_idx) {
            auto item = provider.GetItem(item_idx);
            auto result = TryCollectPoint(gatherer.start_pos, gatherer.end_pos, item.position);
            if (result.IsCollected(gatherer.width + item.width)) {
                gathering_events.emplace_back(
                    item_idx,
                    gatherer_idx,
                    result.sq_distance,
                    result.proj_ratio
                );
            }
        }
    }

    std::sort(gathering_events.begin(), gathering_events.end(),
              [](const GatheringEvent& e_l, const GatheringEvent& e_r) {
                  return e_l.time < e_r.time;
              });

    return gathering_events;
}

}  // namespace collision_detector