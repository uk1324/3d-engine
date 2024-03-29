#include "IntersectLineSegments.hpp"
#include <queue>
#include <set>
#include <variant>
#include "Overloaded.hpp"

// TODO: Maybe do https://www.ricam.oeaw.ac.at/files/reports/11/rep11-21.pdf

/*
Desmos fails do detect intersections (i think, not sure when it considers something an intersection, for example sometimes when graph a is highlighted then an intersection with b is displayed, but when b is highlighted the intersection with a is not displayed) for example xy=2 and xy = 0 and y = x
*/

void intersectLineSegments(std::span<const Vec2> segmentsA, std::span<const Vec2> segmentsB, std::vector<Vec2>& intersectionsOut) {
}

//void intersectLineSegments(std::span<const Vec2> segmentsA, std::span<const Vec2> segmentsB, std::vector<Vec2>& intersectionsOut) {
//
//	struct Segment {
//		i32 indexInArray;
//		i32 arrayIndex;
//
//		bool operator==(const Segment&) const = default;
//	};
//
//	struct LeftEndpointEvent {
//		Segment segment;
//		Vec2 endpoint;
//	};
//
//	struct RightEndpointEvent {
//		Segment segment;
//		float leftEndpointY;
//		Vec2 endpoint;
//	};
//
//	struct IntersectionEvent {
//		Segment segmentA;
//		Segment segmentB;
//		float intersectionX;
//	};
//
//	using Event = std::variant<LeftEndpointEvent, RightEndpointEvent, IntersectionEvent>;
//
//	auto segmentIntersection = [](const Segment& a, const Segment& b) -> std::optional<Vec2> {
//
//	};
//
//	std::priority_queue<Event> events;
//	struct OrderElement {
//		float y = -1.0f; // This can't be stored in the object it would have to be dynamics with would probably require storing the current x in some global variable like in  
//		// https://github.com/maiconpml/bentley-ottmann/blob/main/BentleyOttmann.cpp
//		// Then the y for x would need to be computed dynamically.
//		// Also this technically can invalidate the container, but it shouldn't given how the algorithm works that is how the order changes at endpoints and intersection points. Another issue is implementing the swapping in the intersection case. You can't just change the elements in the order, because that would invalidate it so you have to do something different.
//		Segment segment;
//
//		bool operator==(const OrderElement& other) const {
//			return segment == other.segment;
//		}
//
//		bool operator<(const OrderElement& other) const {
//			return y < other.y;
//		}
//	};
//	// Computational geometry and introduction.
//	// https://www.cs.kent.edu/~dragan/CG/CG-Book.pdf
//
//	// This algorithm could be considered an extension of an algorithm for finding intersection in a set of intervals of real numbers. The simplest algorithm just checks wether each pair of segments intersects which has time complexity O(n^2). A way to speed up the algorithm is to sort the endpoints. Then if no intervals intersect then a linear scan over the sorted segments would look like this S0, E0, S1, E1, where S is the start and E is the end of the interval. Sorting takes O(n*log(n)) and the scan takes O(n) time so in total it takes O(n*log(n)) time.
//
//	// For a given x0 this defines an ordering of the segment based on the y value at that x0 (the intersection of the segment with the line x = x0. 
//	// A nescessary condition for 2 segments to intersect is that they occur consequitively in this order. Because of this the intersections only have to be checked in places where the order changes.
//	// The order only changes in 3 situations:
//	// - when a new endpoint appears at x0, 
//	// - when a endpoint disappears at x0,
//	// - when there is an intersection at x0. then the intersecting segments exchange places in the order
//	std::set<OrderElement> activeOrder;
//	struct Intersection {
//		Segment segmentA;
//		Segment segmentB;
//	};
//	std::vector<Intersection> intersections;
//	while (!events.empty()) {
//		const auto event = events.top();
//		events.pop();
//
//		std::visit(overloaded{
//			[&](const LeftEndpointEvent& e) {
//				const auto it = activeOrder.insert({.y = e.endpoint.y, .segment = e.segment }).first;
//				if (it != activeOrder.end()) {
//					auto above = it;
//					++above;
//					const auto intersection = segmentIntersection(e.segment, above->segment);
//					if (intersection.has_value()) {
//						intersections.push_back({ e.segment, above->segment });
//					}
//				}
//				if (it != activeOrder.begin()) {
//					auto below = it;
//					--below;
//					const auto intersection = segmentIntersection(e.segment, below->segment);
//					if (intersection.has_value()) {
//						intersections.push_back({ e.segment, below->segment });
//					}
//				}
//
//			},
//			[&](const RightEndpointEvent& e) {
//				const OrderElement orderElement{.segment = e.segment };
//				auto it = activeOrder.find(orderElement);
//				if (it != activeOrder.begin() && it != activeOrder.end()) {
//					auto above = it;
//					++above;
//					auto below = it;
//					--below;
//					const auto intersection = segmentIntersection(above->segment, below->segment);
//					const auto intersectsToTheRightOfSegment = intersection.has_value() && intersection->x > e.endpoint.x;
//					// If the intersection is to the left it should have already been detected so don't add it twice.
//					if (intersectsToTheRightOfSegment) {
//						intersections.push_back({ above->segment, below->segment });
//					}
//					activeOrder.erase(it);
//				}
//			},
//			[&](const IntersectionEvent& e) {
//				const auto a = OrderElement{ .segment = e.segmentA };
//				const auto b = OrderElement{ .segment = e.segmentB };
//				auto itA = activeOrder.find(a);
//				auto itB = activeOrder.find(b);
//				//auto& x = *itA;
//				//std::swap(itA->segment, itB->segment);
//				//std::swap();
//				//itA->segment 
//
//				activeOrder.erase(itA);
//				activeOrder.erase(itB);
//				a.y = 
//			},
//		}, event);
//	}
//}
