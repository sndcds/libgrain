//
//  Viewport.cpp
//
//  Created by Roald Christesen on from 09.04.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Views/Viewport.hpp"
#include "App/App.hpp"
#include "Graphic/GraphicContext.hpp"
#include "GUI/Event.hpp"
#include "Bezier/Bezier.hpp"


namespace Grain {

// Menu* Viewport::gBezierValueCurveContextMenu = nullptr; TODO: !!!!

ViewportRuler::ViewportRuler(int32_t group, double pos, float alpha, float size, bool vertical, bool visible) noexcept {
	group_ = group;
	pos_ = pos;
	alpha_ = alpha;
	size_ = size;
	vertical_flag_ = vertical;
	visible_flag_ = visible;
}


Viewport::Viewport(const Rectd& rect, int32_t tag) noexcept : View(rect, tag) {
	type_ = ComponentType::Viewport;
    rulers_.reserve(10);
	setKeepProportions(true);
	setRange({ -1000, -1000, 1000, 1000 });

	_bezierValueCurveBuildContextMenu();
}


Viewport::~Viewport() noexcept {
	GRAIN_RELEASE(bezier_value_curve_);
	// delete g_bezier_value_curve_context_menu; // TODO: !!!
}


Viewport *Viewport::add(View* view, const Rectd& rect) {
    return (Viewport *)Component::addComponentToView((Component *)new Viewport(rect), view, Component::AddFlags::kWantsLayer);
}


void Viewport::updateBeforeDrawing(const Rectd& dirty_rect) noexcept {
	if (fit_mode_ == ContentFitMode::Range) {
		if (range_.width() > std::numeric_limits<float>::min() &&
			range_.height() > std::numeric_limits<float>::min()) {
			scale_.x_ = rect_.width_ / range_.width();
			scale_.y_ = rect_.height_ / range_.height();
			offs_.x_ = range_.min_x_ * scale_.x_;
			offs_.y_ = range_.min_y_ * scale_.y_;
			}
	}
	else if (fit_mode_ == ContentFitMode::Contain) {
		if (range_.width() > std::numeric_limits<float>::min() &&
			range_.height() > std::numeric_limits<float>::min()) {
			double scale_x = rect_.width_ / range_.width();
			double scale_y = rect_.height_ / range_.height();
			double scale = std::min(scale_x, scale_y);
			scale_.x_ = scale;
			scale_.y_ = scale;
			double used_width  = range_.width() * scale;
			double used_height = range_.height() * scale;
			offs_.x_ = -(rect_.x_ + (rect_.width_ - used_width) * 0.5 - range_.min_x_ * scale);
			offs_.y_ = -(rect_.y_ + (rect_.height_ - used_height) * 0.5 - range_.min_y_ * scale);
		}
	}
}


void Viewport::draw(const Rectd& dirty_rect) noexcept {
	auto gc = graphicContextPtr();
	if (!gc) { return; }

	gc->save();
	gc->translate(-offs_);
	gc->scale(scale_);

	auto style = App::guiStyleAtIndex(style_index_);
	if (style) {
		Rectd bounds_rect = boundsRect();

		if (fills_bg_) {
			gc->setFillRGBA(style->viewColor());
			gc->fillRect(bounds_rect);
		}
		else {
			gc->setFillClearColor();
			gc->fillRect(bounds_rect);
		}
	}

	callDrawFunction(gc);

	gc->restore();
}


void Viewport::updateAtMouseDown(const Event& event) noexcept {
	if (is_enabled_) {
		pivot_.x_ = xFromView(event.mouseDownX());
		pivot_.y_ = yFromView(event.mouseDownY());
		remember();
		needsDisplay();
	}
}


void Viewport::handleMouseDown(const Event& event) noexcept {
}


void Viewport::handleMouseDrag(const Event& event) noexcept {
	dragZoom(event);
}


void Viewport::handleMouseUp(const Event& event) noexcept {
	needsDisplay();
}


void Viewport::handleScrollWheel(const Event& event) noexcept {
	double x_amount = -event.deltaX() * (rangeWidth() / width()) * App::scrollWheelSpeed();
	double y_amount = -event.deltaY() * (rangeHeight() / height()) * App::scrollWheelSpeed();

	if (!isFlippedView()) {
		y_amount = -y_amount;
	}

	if (x_scroll_enabled_) {
		if (y_scroll_enabled_) {
			range_.scroll(x_amount, y_amount);
		}
		else {
			range_.scrollX(x_amount);
		}
	}
	if (y_scroll_enabled_) {
		range_.scrollY(y_amount);
	}

	if (x_scroll_enabled_ || y_scroll_enabled_) {
		action_type_ = ActionType::ViewportChanged;
		fireActionAndDisplay(ActionType::None, nullptr);
	}
}


void Viewport::handleMagnification(const Event& event) noexcept {
	if (x_zoom_enabled_ && y_zoom_enabled_) {
		range_.zoom(posFromView(event.mousePos()), event.value());
	}
	else if (x_zoom_enabled_) {
		range_.zoomX(xFromView(event.mouseX()), event.value());
	}
	else if (y_zoom_enabled_) {
		range_.zoomY(yFromView(event.mouseY()), event.value());
	}

	if (x_zoom_enabled_ || y_zoom_enabled_) {
		action_type_ = ActionType::ViewportChanged;
		fireActionAndDisplay(ActionType::None, nullptr);
	}
}


void Viewport::startDragRect(const Vec2d& pos) noexcept {
    drag_rect_.zero();
    drag_pos_1_ = pos;
}


void Viewport::updateDragRect(const Vec2d& pos) noexcept {
    drag_pos_2_ = pos;
    drag_rect_.setPos(drag_pos_1_);
    drag_rect_.setPos2(drag_pos_2_);
    drag_rect_.makePositiveSize();
    needsDisplay();
}


void Viewport::geometryChanged() noexcept {
    if (width() > 0.0 && height() > 0.0) {
        double w = range_.width() / width();
        double h = range_.height() / height();
        double range = range_.width() / w * h;
        double cx = range_.centerX();
        range_.min_x_ = cx - range * 0.5;
        range_.max_x_ = cx + range * 0.5;
        needsDisplay();
    }
}


void Viewport::setRange(const RangeRectd& range) noexcept {
	range_ = range;
	fireActionAndDisplay(ActionType::None, nullptr);
}


void Viewport::zoom(double zoom) noexcept {
	range_.zoom(zoom);
	action_type_ = ActionType::ViewportChanged;
	fireActionAndDisplay(ActionType::None, nullptr);
}


void Viewport::zoomX(double zoom_x) noexcept {
	range_.zoomX(zoom_x);
	action_type_ = ActionType::ViewportChanged;
	fireActionAndDisplay(ActionType::None, nullptr);
}


void Viewport::zoomY(double zoom_y) noexcept {
	range_.zoomY(zoom_y);
	action_type_ = ActionType::ViewportChanged;
	fireActionAndDisplay(ActionType::None, nullptr);
}


void Viewport::clearBG(GraphicContext& gc, const RGB& color) noexcept {
    gc.setFillRGB(color);
    gc.fillRect(boundsRect());
}


double Viewport::widthFromView(double width) const noexcept {
	return width / scale_.x_;
}


double Viewport::heightFromView(double height) const noexcept {
	return height / scale_.y_;
}


double Viewport::xFromView(double x) const noexcept {
	return (x + offs_.x_) / scale_.x_;
}


double Viewport::yFromView(double y) const noexcept {
	return (y + offs_.y_) / scale_.y_;
}


Vec2d Viewport::posFromView(const Vec2d& pos) const noexcept {
	return { xFromView(pos.x_), yFromView(pos.y_) };
}


Rectd Viewport::rectFromView(const Rectd& rect) const noexcept {
	return Rectd(
		xFromView(rect.x_),
		yFromView(rect.y_),
		widthFromView(rect.width_),
		heightFromView(rect.height_));
}


double Viewport::widthToView(double width) const noexcept {
	return width * scale_.x_;
}


double Viewport::heightToView(double height) const noexcept {
	return height * scale_.y_;
}


double Viewport::xToView(double x) const noexcept {
	return x * scale_.x_ - offs_.x_;
}


double Viewport::yToView(double y) const noexcept {
	return y * scale_.y_ - offs_.y_;
}


Vec2d Viewport::posToView(const Vec2d& pos) const noexcept {
	return { xToView(pos.x_), yToView(pos.y_) };
}


Vec2d Viewport::posToView(double x, double y) const noexcept {
	return { xToView(x), yToView(y) };
}


void Viewport::transformPosToView(Vec2d& pos) const noexcept {
	pos.x_ = xToView(pos.x_);
	pos.y_ = yToView(pos.y_);
}


Rectd Viewport::rectToView(const Rectd& rect) const noexcept {
	return { xToView(rect.x_), yToView(rect.y_), widthToView(rect.width_), heightToView(rect.height_) };
}


Rectd Viewport::rectToView(double x, double y, double width, double height) const noexcept {
	return { xToView(x), yToView(y), widthToView(width), heightToView(height) };
}


void Viewport::transformRectToView(Rectd& rect) const noexcept {
	rect.x_ = xToView(rect.x_);
	rect.y_ = yToView(rect.y_);
	rect.width_ = widthToView(rect.width_);
	rect.height_ = heightToView(rect.height_);
}


void Viewport::bezierToView(Bezier& bezier) const noexcept {
	for (int32_t i = 0; i < 4; i++) {
		transformPosToView(bezier.pos_[i]);
	}
}


double Viewport::hitBezier(const Bezier& bezier, const Vec2d& pos, double radius) const noexcept {
	Bezier b = bezier;
	bezierToView(b);
	return b.hit(pos, radius);
}


void Viewport::deltaFromView(const Vec2d& delta, Vec2d& out_delta) const noexcept {
	out_delta = posFromView(posToView(0.0f, 0.0f) + delta);
}


Vec2d Viewport::deltaFromView(const Vec2d& delta) const noexcept {
	Vec2d result_delta;
	deltaFromView(delta, result_delta);
	return result_delta;
}


void Viewport::dragZoom(const Event& event) noexcept {
	if (fit_mode_ == ContentFitMode::Contain) {

	}
	else if (fit_mode_== ContentFitMode::Range) {
		double zoom_factor = 1.0;
		RangeRectd old_range = range_;

		if (keep_proportions_) {
			double delta_x = event.mouseDragDeltaX();
			zoom_factor = delta_x >= 0.0 ? 1.0 / (1.0 + delta_x / zoom_step_.x_) : 1.0 - delta_x / zoom_step_.x_;
		}

		if (x_zoom_enabled_) {
			if (!keep_proportions_) {
				double delta_x = event.mouseDragDeltaX();
				zoom_factor = delta_x >= 0.0f ? 1.0f / (1.0f + delta_x / zoom_step_.x_) : 1.0f - delta_x / zoom_step_.x_;
			}

			double px = pivot_.x_;
			double min_x = px + (rem_range_.min_x_ - px) * zoom_factor;
			double max_x = px + (rem_range_.max_x_ - px) * zoom_factor;

			if (max_x - min_x > 0.001) {
				range_.min_x_ = min_x;
				range_.max_x_ = max_x;
			}
		}

		if (y_zoom_enabled_) {
			if (!keep_proportions_) {
				double delta_y = event.mouseDragDeltaY();
				if (isFlippedView()) {
					delta_y = -delta_y;
				}
				zoom_factor = delta_y >= 0.0 ? 1.0 / (1.0 + delta_y / zoom_step_.y_) : 1.0 - delta_y / zoom_step_.y_;
			}

			double py = pivot_.y_;
			double minY = py + (rem_range_.min_y_ - py) * zoom_factor;
			double maxY = py + (rem_range_.max_y_ - py) * zoom_factor;
			if (maxY - minY > 0.001) {
				range_.min_y_ = minY;
				range_.max_y_ = maxY;
			}
		}

		if (range_ != old_range) {
			action_type_ = ActionType::ViewportChanged;
			fireActionAndDisplay(ActionType::None, nullptr);
		}
	}
}


void Viewport::addRuler(int32_t group, double pos, float alpha, float size, bool vertical, bool visible) noexcept {
	rulers_.push(ViewportRuler(group, pos, alpha, size, vertical, visible));
}


void Viewport::setBezierValueCurve(BezierValueCurve* bezier_value_curve) noexcept {
	if (bezier_value_curve != bezier_value_curve_) {
		GRAIN_RELEASE(bezier_value_curve_);
		GRAIN_RETAIN(bezier_value_curve);
		bezier_value_curve_ = bezier_value_curve;
	}
}


void Viewport::_getBezierPointPositionData(
	BezierValueCurvePoint* point,
	Vec2d& pos,
	Vec2d& left_pos,
	Vec2d& right_pos
) const noexcept {
	pos = posToView(point->pos_);
	left_pos = point->usesLeftControl() ? posToView(point->used_left_pos_) : pos;
	right_pos = point->usesRightControl() ? posToView(point->used_right_pos_) : pos;
}


void Viewport::drawBezierValueCurve(
	const Rectd& dirty_rect,
	BezierValueCurve* bezier_value_curve,
	BezierValueCurveDrawSettings& bezier_value_curve_draw_settings
) noexcept {
	if (!bezier_value_curve) {
		return;
	}

	GraphicContext gc(this);
    gc.save();

	Rectd bounds_rect = this->boundsRect();

	bool enabled = is_enabled_ && bezier_value_curve_draw_settings.isEnabled();
	bool standard_mode = false;
	bool envelope_mode = false;
	bool normal_scale_flag = false;
	bool pitch_scale_flag = false;
	bool level_scale_flag = false;		// Dezibel scale
	bool dynamic_scale_flag = false;		// Todo ...
	bool fill_flag = false;

	switch (bezier_value_curve->mode()) {
		case BezierValueCurve::Mode::Standard:
			standard_mode = true;
			normal_scale_flag = enabled;
			break;
		case BezierValueCurve::Mode::Envelope:
			normal_scale_flag = enabled;
			envelope_mode = true;
			fill_flag = true;
			break;
		case BezierValueCurve::Mode::Filter:
			normal_scale_flag = enabled;
			break;
		default:
			break;
	}

	RGB grid_color = { 0, 0, 0 };	// TODO: uiColor(Gr::UI_COLOR_GRID);
	RGB sub_grid_color = { 0, 0, 0 };	// TODO: uiColor(Gr::UI_COLOR_SUBGRID);
	RGB bg_color = { 0, 0, 0 };	// TODO: uiColor(Gr::UI_COLOR_EDITOR_BG);
	RGB editor_color1 = { 0, 0, 0 };	// TODO: uiColor(Gr::UI_COLOR_EDITOR_COLOR1);
	RGB stroke_color;
	RGB point_colors[6];
	RGB control_line_color;
	RGB light_key_color(1.0f);
	RGB dark_key_color(0.2f);

	float stroke_width = bezier_value_curve_draw_settings.stroke_width_;
	float stroke_alpha = bezier_value_curve_draw_settings.alpha_;
	float point_alpha = bezier_value_curve_draw_settings.point_alpha_;
	float fill_alpha = bezier_value_curve_draw_settings.fill_alpha_;
	double point_radius = bezier_value_curve_draw_settings.point_radius_;
	double selected_point_radius = bezier_value_curve_draw_settings.active_point_radius_;
	double control_radius = bezier_value_curve_draw_settings.control_radius_;

	float keyboard_alpha = 0.06f;

	if (enabled) {
		stroke_color = { 0, 0, 0 }; // TODO: uiColor(Gr::UI_COLOR_EDITOR_FG);
		control_line_color = { 0, 0, 0 }; // TODO: uiColor(Gr::UI_COLOR_EDITOR_FG);
		stroke_alpha = bezier_value_curve_draw_settings.alpha();

		for (int32_t i = 0; i < 6; i++) {
			point_colors[i] = { 0, 0, 0 }; // TODO: bezier_value_curve_draw_settings.m_c[i];
		}
	}
	else {
		stroke_color = { 0, 0, 0 }; // TODO: uiColor(Gr::UI_COLOR_EDITOR_FG);	// Todo ...
	}

	double root_freq = 20.0;
	double top_freq = (1024.0 - 1.0) * root_freq;

	if (bezier_value_curve_draw_settings.shows_keyboard_) {
        gc.drawHorizontalKeyboard(21, 108, 60, root_freq, top_freq, xToView(0), xToView(1), 0, bounds_rect.height_, light_key_color, dark_key_color, bg_color, editor_color1, keyboard_alpha);
	}

	if (normal_scale_flag) {
		Rectd line_rect(0, 0, bounds_rect.width_, 1);
		line_rect.y_ = yToView(0) - 0.5;
        gc.setFillRGB(grid_color);
        gc.fillRect(line_rect);
		line_rect.y_ = yToView(1) - 0.5;
        gc.fillRect(line_rect);
		line_rect.y_ = yToView(0.25) - 0.5;
        gc.setFillRGB(sub_grid_color);
        gc.fillRect(line_rect);
		line_rect.y_ = yToView(0.5) - 0.5;
        gc.fillRect(line_rect);
		line_rect.y_ = yToView(0.75) - 0.5;
        gc.fillRect(line_rect);

		if (envelope_mode && bezier_value_curve) {
			BezierValueCurvePoint* decay_point = bezier_value_curve->decayBeginPoint();
            if (decay_point) {
                if (decay_point->isDecayBegin()) {
                    line_rect.set(xToView(decay_point->pos().x_) - 0.5, 0, 1, bounds_rect.height_);
                    gc.fillRect(line_rect);
                }
			}
		}
	}
	else if (dynamic_scale_flag) {	// TODO: !!!!
	}

	// Draw the curve
	bezier_value_curve_->_update();
    Bezier::PointType a_point_type = Bezier::PointType::Undefined;
    Bezier::PointType b_point_type = Bezier::PointType::Undefined;
	Vec2d a_pos, al_pos, ar_pos;
	Vec2d b_pos, bl_pos, br_pos;
	int32_t index = 0;

	// Fill
	if (fill_flag) {
		// TODO: !!!!
	}

	// Control tangents and control points
	{
        gc.setStrokeRGB(stroke_color);
        gc.setFillRGB(bg_color);
        gc.setStrokeWidth(1);

		index = 0;
        int32_t last_index = bezier_value_curve->lastPointIndex();

        for (auto& point : bezier_value_curve->points()) {
			if (point.isSelected()) {
				a_point_type = point.point_type_;
				a_pos = posToView(point.pos_);

				if (index > 0 && point.hasLeftControl()) {
					al_pos = posToView(point.left_pos_);
                    gc.setStrokeRGBAndAlpha(stroke_color, stroke_alpha / 2);
                    gc.strokeLine(a_pos, al_pos);
//					gc.fillCircle(alPos, controlRadius);
                    gc.strokeCircle(al_pos, control_radius);
				}

				if (index < last_index && point.hasRightControl()) {
					ar_pos = posToView(point.right_pos_);
                    gc.setStrokeRGBAndAlpha(stroke_color, stroke_alpha / 2);
                    gc.strokeLine(a_pos, ar_pos);
//					gc.fillCircle(arPos, controlRadius);
                    gc.strokeCircle(ar_pos, control_radius);
				}
			}

			index++;
		}
	}

	// Stroke
	index = 0;
    for (auto& point : bezier_value_curve->points()) {
		if (index == 0) {
			b_point_type = point.point_type_;
			_getBezierPointPositionData(&point, b_pos, bl_pos, br_pos);
		}
		else {
			a_point_type = b_point_type;
			a_pos = b_pos;
			al_pos = bl_pos;
			ar_pos = br_pos;

			b_point_type = point.point_type_;
			_getBezierPointPositionData(&point, b_pos, bl_pos, br_pos);

            gc.moveTo(a_pos);
			if (a_point_type == Bezier::PointType::Linear && b_point_type == Bezier::PointType::Linear) {
                gc.lineTo(b_pos);
			}
			else {
                gc.curveTo(ar_pos, bl_pos, b_pos);
			}

            gc.setStrokeRGBAndAlpha(stroke_color, stroke_alpha);
            gc.setStrokeWidth(stroke_width);
            gc.setStrokeJoinStyle(StrokeJoinStyle::Bevel);
            gc.setStrokeCapStyle(StrokeCapStyle::Round);
            gc.strokePath();
		}

		index++;
	}

	// Points
	{
		index = 0;
        for (auto& point : bezier_value_curve->points()) {
            gc.setFillRGBAndAlpha(point_colors[(int32_t)point.point_type_], point_alpha);
            gc.fillCircle(posToView(point.pos_), point.isSelected() ? selected_point_radius : point_radius);
			index++;
		}
	}

    gc.restore();
}


void Viewport::bezierValueCurveSetTypeOfSelectedPoints(Bezier::PointType point_type) noexcept {
	if (bezier_value_curve_) {
		if (bezier_value_curve_->setTypeOfSelectedPoints(point_type)) {
			bezier_value_curve_->mustUpdate();
			bezierValueCurveUpdateAndAction(true);
		}
	}
}


void Viewport::bezierValueCurveHandleMouseDown(const Event& event) noexcept {
	if (bezier_value_curve_ == nullptr) {
		event.mousePressedFinished();
		return;
	}

	bezier_value_curve_modification_count_at_mouse_down_ = bezier_value_curve_->modificationCount();
	bezier_value_curve_drag_point_ = nullptr;
	bezier_value_curve_drag_mode_ = BezierValueCurveDragMode::Nothing;

	Vec2d mouse_down_pos = event.mouseDownPos();

	if (event.isAltPressedOnly()) {
		remember();
		bezier_value_curve_drag_mode_ = BezierValueCurveDragMode::ZoomView;
		return;
	}

    BezierValueCurvePoint* hit_point = nullptr;
    BezierValueCurvePoint::Part part;

    int32_t hit_point_index = bezierValueCurveHitPointIndex(mouse_down_pos, 8.0f, part);
    if (hit_point_index >= 0) {
        hit_point = bezier_value_curve_->mutPointAtIndex(hit_point_index);
        if (hit_point != nullptr) {
            if (part == BezierValueCurvePoint::Part::Point) {
                if (event.isShiftPressed()) {
                    hit_point->invertSelection();
                }
                else if (!hit_point->isSelected()) {
                    bezier_value_curve_->deselectAllPoints();
                    hit_point->select();
                }

				bezier_value_curve_remembered_mouse_drag_origin_ = posToView(hit_point->pos_);
            }
            else if (part == BezierValueCurvePoint::Part::Left) {
                bezier_value_curve_remembered_mouse_drag_origin_ = posToView(hit_point->left_pos_);
            }
            else if (part == BezierValueCurvePoint::Part::Right) {
                bezier_value_curve_remembered_mouse_drag_origin_ = posToView(hit_point->right_pos_);
            }
        }
	}
	else {
		// Does the mouse hit the curve?
		for (int32_t i = 1; i < bezier_value_curve_->length(); i++) {
			BezierValueCurvePoint* point_a = bezier_value_curve_->mutPointAtIndex(i - 1);
			BezierValueCurvePoint* point_b = bezier_value_curve_->mutPointAtIndex(i);

			Bezier bezier(*point_a, *point_b);
			bezierToView(bezier);
			double t = bezier.hit(mouse_down_pos, 6.0);
			if (t > 0.0f && t < 1.0f) {
				// Yes, it hits ... split the curve segment into two segments
				bezier_value_curve_->deselectAllPoints();
				if (bezier_value_curve_->split(i - 1, t, true)) {
					bezierValueCurveUpdateAndAction(true);
				}
				return;
			}
		}

		if (!event.isShiftPressed()) {
			bezier_value_curve_->deselectAllPoints();
		}
	}

	bezier_value_curve_drag_point_ = nullptr;
	needsDisplay();

	bezier_value_curve_->rememberAllPoints();

    if (hit_point != nullptr) {
        bezier_value_curve_drag_point_ = hit_point;
        bezier_value_curve_drag_point_part_ = part;
        bezier_value_curve_drag_point_->startMouseAction(bezier_value_curve_drag_point_part_, *this);
    	bezier_value_curve_point_remembered_pos_in_view_ = posToView(hit_point->pos_);
        bezier_value_curve_point_remembered_left_pos_in_wiew_ = posToView(hit_point->left_pos_);
        bezier_value_curve_point_remembered_right_pos_in_wiew_ = posToView(hit_point->right_pos_);
        bezier_value_curve_point_remembered_left_length_ = bezier_value_curve_point_remembered_left_pos_in_wiew_.distance(bezier_value_curve_point_remembered_pos_in_view_);
        bezier_value_curve_point_remembered_right_length_ = bezier_value_curve_point_remembered_right_pos_in_wiew_.distance(bezier_value_curve_point_remembered_pos_in_view_);
    }
	else {
		startDragRect(event.mouseDownPos());
		bezier_value_curve_drag_mode_ = BezierValueCurveDragMode::Select;
	}
}


void Viewport::bezierValueCurveHandleMouseDrag(const Event& event) noexcept {
	Vec2d mouse_pos = event.mousePos();

	if (bezier_value_curve_drag_mode_ == BezierValueCurveDragMode::Select) {
        updateDragRect(mouse_pos);
		bezierValueCurveSelectInRect(drag_rect_);
		needsDisplay();
		return;
	}

	if (bezier_value_curve_drag_mode_ == BezierValueCurveDragMode::ZoomView) {
		dragZoom(event);
		return;
	}

	Vec2d mouse_delta = event.mouseDragDelta();

	if (event.isShiftPressedOnly()) {
		if (mouse_delta.isHorizontal()) {
			mouse_delta.y_ = 0.0f;
		}
		else {
			mouse_delta.x_ = 0.0f;
		}
	}

	Vec2d new_pos_in_view = bezier_value_curve_remembered_mouse_drag_origin_ + mouse_delta;

	if (bezier_value_curve_drag_point_) {
		bezier_value_curve_drag_mode_ = BezierValueCurveDragMode::Point;

		if (bezier_value_curve_drag_point_part_ == BezierValueCurvePoint::Part::Point) {
			Vec2d delta = deltaFromView(mouse_delta);
			if (bezier_value_curve_->moveRememberedSelectedPoints(delta))
				bezierValueCurveUpdateAndAction();
		}
		else if (bezier_value_curve_drag_point_part_ == BezierValueCurvePoint::Part::Left) {
			// First hit and control key pressed?
			if (event.mouseDragCount() == 1 && event.isControlPressedOnly()) {
                if (bezier_value_curve_drag_point_->pointType() != Bezier::PointType::Corner) {
                    bezier_value_curve_drag_point_->setPointType(Bezier::PointType::Corner);
                }
                else {
                    bezier_value_curve_drag_point_->setPointType(Bezier::PointType::Smooth1);
                }
			}

			bezier_value_curve_drag_point_->left_ = posFromView(new_pos_in_view) - bezier_value_curve_drag_point_->pos_;

			if (bezier_value_curve_drag_point_->isSmooth()) {
                // Adjust the right (opposite) control point.
                Vec2d tangentInView = -(new_pos_in_view - bezier_value_curve_point_remembered_pos_in_view_);
                if (bezier_value_curve_drag_point_->point_type_ == Bezier::PointType::Smooth1) {
                    tangentInView.setLength(bezier_value_curve_point_remembered_right_length_);
				}
                Vec2d rightInView = bezier_value_curve_point_remembered_pos_in_view_ + tangentInView;
				bezier_value_curve_drag_point_->right_ = posFromView(rightInView) - bezier_value_curve_drag_point_->pos_;
			}

			bezier_value_curve_->mustUpdate();
			bezierValueCurveUpdateAndAction();
		}
		else if (bezier_value_curve_drag_point_part_ == BezierValueCurvePoint::Part::Right) {
			// First hit and control key pressed?
			if (event.mouseDragCount() == 1 && event.isControlPressedOnly()) {
                if (bezier_value_curve_drag_point_->pointType() != Bezier::PointType::Corner) {
                    bezier_value_curve_drag_point_->setPointType(Bezier::PointType::Corner);
                }
                else {
                    bezier_value_curve_drag_point_->setPointType(Bezier::PointType::Smooth1);
                }
			}

			bezier_value_curve_drag_point_->right_ = posFromView(new_pos_in_view) - bezier_value_curve_drag_point_->pos_;

			if (bezier_value_curve_drag_point_->isSmooth()) {
                // Adjust the left (opposite) control point.
                Vec2d tangentInView = -(new_pos_in_view - bezier_value_curve_point_remembered_pos_in_view_);
                if (bezier_value_curve_drag_point_->point_type_ == Bezier::PointType::Smooth1) {
                    tangentInView.setLength(bezier_value_curve_point_remembered_left_length_);
                }
                Vec2d leftInView = bezier_value_curve_point_remembered_pos_in_view_ + tangentInView;
                bezier_value_curve_drag_point_->left_ = posFromView(leftInView) - bezier_value_curve_drag_point_->pos_;
			}

			bezier_value_curve_->mustUpdate();
			bezierValueCurveUpdateAndAction();
		}
	}
}


void Viewport::bezierValueCurveHandleMouseUp(const Event& event) noexcept {
	bezier_value_curve_drag_point_ = nullptr;

	if (bezier_value_curve_drag_mode_ == BezierValueCurveDragMode::Point) {
		if (!continuous_update_flag_) {
			if (bezier_value_curve_modification_count_at_mouse_down_ != bezier_value_curve_->modificationCount())
				action_type_ = ActionType::StateChanged;
			fireAction(ActionType::None, nullptr);
		}
	}

	BezierValueCurveDragMode used_drag_mode = bezier_value_curve_drag_mode_;
	bezier_value_curve_drag_mode_ = BezierValueCurveDragMode::Nothing;

	if (used_drag_mode == BezierValueCurveDragMode::Select)
		needsDisplay();
}


void Viewport::bezierValueCurveHandleRightMouseDown(const Event& event) noexcept {
	Vec2d pos = event.mouseDownPos();
	pos.translate(2, 8);
	// g_bezier_value_curve_context_menu->setAction(_bezierValueCurveMenuAction, this, true); TODO: !!!!
	// g_bezier_value_curve_context_menu->popUp(this, pos); TODO: !!!!
	event.rightMousePressedFinished();
}


void Viewport::bezierValueCurveFitToView() noexcept {

	if (bezier_value_curve_) {
		// m_bezier_value_curve->setViewportToFit(*this, false); TODO: !!!!
		action_type_ = ActionType::ViewportChanged;
		fireActionAndDisplay(ActionType::None, nullptr);
	}
}


int32_t Viewport::bezierValueCurveHitPointIndex(const Vec2d& pos, double radius, BezierValueCurvePoint::Part& out_part) const noexcept {
    int32_t hit_point_index = -1;
	double min_distance = radius;
    int32_t index = 0;
    for (auto& point : bezier_value_curve_->points()) {
		BezierValueCurvePoint::Part part = point.hit(*this, pos, min_distance);
		if (part != BezierValueCurvePoint::Part::None) {
			hit_point_index = index;
			out_part = part;
		}
        index++;
	}

	return hit_point_index;
}


void Viewport::bezierValueCurveSelectInRect(const Rectd& rect) noexcept {
    bezier_value_curve_->selectPointsInRect(rectFromView(rect));
}


bool Viewport::bezierValueCurveUpdate() noexcept {
	if (bezier_value_curve_) {
		if (bezier_value_curve_->_update()) {
			is_modified_since_mouse_down_ = true;
			is_modified_while_mouse_drag_ = true;
			needsDisplay();
			return true;
		}
	}

	return false;
}


void Viewport::bezierValueCurveUpdateAndAction(bool forced) noexcept {
	if (bezierValueCurveUpdate() || forced) {
		if (continuous_update_flag_ || forced) {
			action_type_ = Component::ActionType::StateChanged;
			fireActionAndDisplay(ActionType::None, nullptr);
		}
	}
}


void Viewport::_bezierValueCurveBuildContextMenu() noexcept {
	/* TODO: !!!!
	if (!gBezierValueCurveContextMenu) {
		GrMenu *menu = new GrMenu();
		menu->addItem("Linear", BEZIER_CURVE_MENU_LINEAR);
		menu->addItem("Corner", BEZIER_CURVE_MENU_CORNER);
		menu->addItem("Smooth 1st degree", BEZIER_CURVE_MENU_SMOOTH1);
		menu->addItem("Smooth 2nd degree", BEZIER_CURVE_MENU_SMOOTH2);
		menu->addItem("Continue right", BEZIER_CURVE_MENU_RIGHT);
		menu->addItem("Continue left", BEZIER_CURVE_MENU_LEFT);

		menu->addSeparator();

		menu->addItem("Delete", BEZIER_CURVE_MENU_DELETE);

		menu->addSeparator();
		menu->addItem("Flip vertical", BEZIER_CURVE_MENU_FLIP_VERTICAL);

		GrMenu *subMenu = menu->addSubMenu("Align");
		subMenu->addItem("Top", BEZIER_CURVE_MENU_ALIGN_TOP);
		subMenu->addItem("Center", BEZIER_CURVE_MENU_ALIGN_CENTER);
		subMenu->addItem("Bottom", BEZIER_CURVE_MENU_ALIGN_BOTTOM);
		subMenu->addItem("Zero", BEZIER_CURVE_MENU_ALIGN_ZERO);

		menu->setAction(_bezierValueCurveMenuAction, this);
		subMenu->setAction(_bezierValueCurveMenuAction, this);

		menu->addSeparator();
		menu->addItem("Help", BEZIER_CURVE_MENU_HELP);

		gBezierValueCurveContextMenu = menu;
	}
	*/
}


/* TODO: !!!!
void Viewport::Viewport::_bezierValueCurveMenuAction(Menu *menu, int32_t tag, void *infoPtr) noexcept {
	if (!infoPtr)
		return;

	Viewport *view = (Viewport *)infoPtr;
	if (!view)
		return;

	GrBezierValueCurve *bezierValueCurve = view->getBezierValueCurve();
	if (!bezierValueCurve)
		return;

	switch (tag) {
		case BEZIER_CURVE_MENU_LINEAR:
			view->bezierValueCurveSetTypeOfSelectedPoints(GrBezier::PointType::Linear);
			break;

		case BEZIER_CURVE_MENU_CORNER:
			view->bezierValueCurveSetTypeOfSelectedPoints(GrBezier::PointType::Corner);
			break;

		case BEZIER_CURVE_MENU_SMOOTH1:
			view->bezierValueCurveSetTypeOfSelectedPoints(GrBezier::PointType::Smooth1);
			break;

		case BEZIER_CURVE_MENU_SMOOTH2:
			view->bezierValueCurveSetTypeOfSelectedPoints(GrBezier::PointType::Smooth2);
			break;

		case BEZIER_CURVE_MENU_RIGHT:
			view->bezierValueCurveSetTypeOfSelectedPoints(GrBezier::PointType::Right);
			break;

		case BEZIER_CURVE_MENU_LEFT:
			view->bezierValueCurveSetTypeOfSelectedPoints(GrBezier::PointType::Left);
			break;

		case BEZIER_CURVE_MENU_DELETE:
			bezierValueCurve->removeSelectedPoints();
			view->bezierValueCurveUpdateAndAction();
			break;

		case BEZIER_CURVE_MENU_FLIP_VERTICAL:
			bezierValueCurve->flipVertical();
			view->bezierValueCurveUpdateAndAction();
			break;

		case BEZIER_CURVE_MENU_ALIGN_TOP:
			bezierValueCurve->alignSelectedPoints(GrAlignment::Top);
			view->bezierValueCurveUpdateAndAction();
			break;

		case BEZIER_CURVE_MENU_ALIGN_CENTER:
			bezierValueCurve->alignSelectedPoints(GrAlignment::Center);
			view->bezierValueCurveUpdateAndAction();
			break;

		case BEZIER_CURVE_MENU_ALIGN_BOTTOM:
			bezierValueCurve->alignSelectedPoints(GrAlignment::Bottom);
			view->bezierValueCurveUpdateAndAction();
			break;

		case BEZIER_CURVE_MENU_ALIGN_ZERO:
			bezierValueCurve->alignSelectedPoints(GrAlignment::Zero);
			view->bezierValueCurveUpdateAndAction();
			break;

		case BEZIER_CURVE_MENU_HELP:
			break;
	}
}
*/

} // End of namespace Grain
