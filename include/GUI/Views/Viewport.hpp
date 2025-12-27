//
//  Viewport.hpp
//
//  Created by Roald Christesen on from 09.04.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainViewport_hpp
#define GrainViewport_hpp

#include "Grain.hpp"
#include "2d/Border.hpp"
#include "Math/Vec2.hpp"
#include "2d/RangeRect.hpp"
#include "GUI/Views/View.hpp"
#include "Bezier/Bezier.hpp"
#include "Bezier/BezierValueCurve.hpp"
#include "Type/List.hpp"
// #include "GrMenu.h" TODO: !!!


namespace Grain {

class ViewportRuler { // TODO:!!! Is it used? Should it be used?

public:
	int32_t group_ = 0;
	double pos_ = 0.0;
	float alpha_ = 1.0f;
	float size_ = 1.0f;
	bool vertical_flag_ = true;
	bool visible_flag_ = true;

public:
	ViewportRuler() noexcept = default;
	ViewportRuler(int32_t group, double pos, float alpha, float size, bool vertical, bool visible) noexcept;
};


class Viewport : public View {

public:
	enum class ContentFitMode {
		Cover = 0,
		Contain,
		Range
	};

	enum class BezierValueCurveDragMode {
		Nothing = 0,
		ZoomView,
		Point,
		Select
	};

	enum {
		kBezierCurveMenu_Linear = 1,
		kBezierCurveMenu_Corner,
		kBezierCurveMenu_Smooth1,
		kBezierCurveMenu_Smooth2,
		kBezierCurveMenu_Right,
		kBezierCurveMenu_Left,
		kBezierCurveMenu_Delete,
		kBezierCurveMenu_FlipVertical,
		kBezierCurveMenu_AlignTop,
		kBezierCurveMenu_AlignCenter,
		kBezierCurveMenu_AlignBottom,
		kBezierCurveMenu_AlignZero,
		kBezierCurveMenu_Help
	};

protected:
	ContentFitMode fit_mode_ = ContentFitMode::Range;
	Vec2d scale_{};
	Vec2d offs_{};
	RangeRectd range_{};
	RangeRectd rem_range_{};

	bool keep_proportions_ = true;
	bool x_scroll_enabled_ = true;
	bool y_scroll_enabled_ = true;
	bool x_zoom_enabled_ = true;
	bool y_zoom_enabled_ = true;

	Vec2d zoom_step_{ 100.0, 100.0 };
	Vec2d pivot_{};		///< Used for zooming
    Vec2d drag_pos_1_{};
	Vec2d drag_pos_2_{};
	Rectd drag_rect_{};

	List<ViewportRuler> rulers_; // Todo: Implement rulers

	// Bezier value curve
	BezierValueCurve* bezier_value_curve_ = nullptr;	///< Pointer to current bezier value curve
	float bezier_value_curve_width_ = 2.0f;			///< Width of stroke
	float bezier_value_curve_alpha_ = 0.7f;			///< Transparency of stroke
	float bezier_point_alpha_ = 0.8f;					///< Transparency of points

	BezierValueCurvePoint* bezier_value_curve_drag_point_ = nullptr;	///< Pointer to the control point which is currently dragged
	BezierValueCurvePoint::Part bezier_value_curve_drag_point_part_ = BezierValueCurvePoint::Part::None;
	Vec2d bezier_value_curve_remembered_mouse_drag_origin_{};
	int64_t bezier_value_curve_modification_count_at_mouse_down_{};

	Vec2d bezier_value_curve_point_remembered_pos_in_view_{};
	Vec2d bezier_value_curve_point_remembered_left_pos_in_wiew_{};
	Vec2d bezier_value_curve_point_remembered_right_pos_in_wiew_{};
	double bezier_value_curve_point_remembered_left_length_{};
	double bezier_value_curve_point_remembered_right_length_{};

	BezierValueCurveDragMode bezier_value_curve_drag_mode_ = BezierValueCurveDragMode::Nothing;

	// static Menu* gBezierValueCurveContextMenu;

public:
	explicit Viewport(const Rectd& rect, int32_t tag = 0) noexcept;
	~Viewport() noexcept override;

	[[nodiscard]] const char* className() const noexcept override {
		return "Viewport";
	}

    static Viewport* add(View* view) { return add(view, Rectd()); }
    static Viewport* add(View* view, const Rectd& rect);

	void updateBeforeDrawing(const Rectd& dirty_rect) noexcept override;
	void draw(const Rectd& dirty_rect) noexcept override;

	void updateAtMouseDown(const Event& event) noexcept override;
	void handleMouseDown(const Event& event) noexcept override;
	void handleMouseDrag(const Event& event) noexcept override;
	void handleMouseUp(const Event& event) noexcept override;

	void handleScrollWheel(const Event& event) noexcept override;
	void handleMagnification(const Event& event) noexcept override;

    virtual void startDragRect(const Vec2d& pos) noexcept;
    virtual void updateDragRect(const Vec2d& pos) noexcept;

    void geometryChanged() noexcept override;

	[[nodiscard]] Vec2d scale() const noexcept { return scale_; }
	[[nodiscard]] double scaleX() const noexcept { return scale_.x(); }
	[[nodiscard]] double scaleY() const noexcept { return scale_.y(); }
	[[nodiscard]] RangeRectd range() const noexcept { return range_; }
	[[nodiscard]] double rangeWidth() const noexcept { return range_.width(); }
	[[nodiscard]] double rangeHeight() const noexcept { return range_.height(); }

	void setRange(const RangeRectd& range) noexcept;
	void zoom(double zoom) noexcept;
	void zoomX(double zoom_x) noexcept;
	void zoomY(double zoom_y) noexcept;

	void setKeepProportions(bool keep_proportions) noexcept { keep_proportions_ = keep_proportions; }
	void setScrollEnabled(bool x_flag, bool y_flag) noexcept { x_scroll_enabled_ = x_flag; y_scroll_enabled_ = y_flag; }
	void setZoomEnabled(bool x_flag, bool y_flag) noexcept { x_zoom_enabled_ = x_flag; y_zoom_enabled_ = y_flag; }

	void dragZoom(const Event& event) noexcept;

	void clearBG(GraphicContext& gc, const RGB& color) noexcept;


    [[nodiscard]] double widthFromView(double width) const noexcept;
	[[nodiscard]] double heightFromView(double height) const noexcept;
	[[nodiscard]] double xFromView(double x) const noexcept;
	[[nodiscard]] double yFromView(double y) const noexcept;
	[[nodiscard]] Vec2d posFromView(const Vec2d& pos) const noexcept;
	[[nodiscard]] Rectd rectFromView(const Rectd& rect) const noexcept;

	[[nodiscard]] double widthToView(double width) const noexcept;
	[[nodiscard]] double heightToView(double height) const noexcept;
	[[nodiscard]] double xToView(double x) const noexcept;
	[[nodiscard]] double yToView(double y) const noexcept;
	[[nodiscard]] Vec2d posToView(const Vec2d& pos) const noexcept;
	[[nodiscard]] Vec2d posToView(double x, double y) const noexcept;
	void transformPosToView(Vec2d& pos) const noexcept;

	[[nodiscard]] Rectd rectToView(const Rectd& rect) const noexcept;
	[[nodiscard]] Rectd rectToView(double x, double y, double width, double height) const noexcept;
	void transformRectToView(Rectd &rect) const noexcept;

	void bezierToView(Bezier& bezier) const noexcept;
	[[nodiscard]] double hitBezier(const Bezier& bezier, const Vec2d& pos, double radius) const noexcept;


	void deltaFromView(const Vec2d& delta, Vec2d& out_delta) const noexcept;
	[[nodiscard]] Vec2d deltaFromView(const Vec2d& delta) const noexcept;

	void remember() noexcept{ rem_range_ = range_; }

	void addRuler(int32_t group, double pos, float alpha, float size, bool vertical, bool visible) noexcept;


	void setBezierValueCurve(BezierValueCurve* bezier_value_curve) noexcept;

	[[nodiscard]] BezierValueCurve* mutBezierValueCurve() const noexcept { return bezier_value_curve_; }

	void _getBezierPointPositionData(BezierValueCurvePoint* point, Vec2d& pos, Vec2d& left_pos, Vec2d& right_pos) const noexcept;

	void drawBezierValueCurve(const Rectd& dirty_rect, BezierValueCurve* bezier_value_curve, BezierValueCurveDrawSettings& bezier_value_curve_draw_settings) noexcept;

	void bezierValueCurveSetTypeOfSelectedPoints(Bezier::PointType point_type) noexcept;

	void bezierValueCurveHandleMouseDown(const Event& event) noexcept;
	void bezierValueCurveHandleMouseDrag(const Event& event) noexcept;
	void bezierValueCurveHandleMouseUp(const Event& event) noexcept;
	void bezierValueCurveHandleRightMouseDown(const Event& event) noexcept;
	void bezierValueCurveFitToView() noexcept;
	int32_t bezierValueCurveHitPointIndex(const Vec2d& pos, double radius, BezierValueCurvePoint::Part &out_part) const noexcept;
	void bezierValueCurveSelectInRect(const Rectd &rect) noexcept;
	bool bezierValueCurveUpdate() noexcept;
	void bezierValueCurveUpdateAndAction(bool forced = false) noexcept;

	void _bezierValueCurveBuildContextMenu() noexcept;
	// static void _bezierValueCurveMenuAction(Menu *menu, int32_t tag, void *info_ptr) noexcept; // TODO: !!!
};


} // End of namespace Grain

#endif // GrainViewport_hpp
