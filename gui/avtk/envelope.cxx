/*
 * Copyright(c) 2016, OpenAV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENAV BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "envelope.hxx"

#include <stdio.h>
#include "ui.hxx"
#include "theme.hxx"


using namespace Avtk;

Envelope::Envelope( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Widget( ui, x_, y_, w_, h_, label_ )
{
	dragMode( DM_DRAG_VERTICAL );

	a = 0.f;
	d = s = r = 0.5f;
}

void Envelope::draw( cairo_t* cr )
{
	cairo_save( cr );

	roundedBox(cr, x_, y_, w_, h_, theme_->cornerRadius_ );
	theme_->color( cr, BG_DARK );
	cairo_fill_preserve(cr);
	theme_->color( cr, FG );
	cairo_set_line_width(cr, 1.4);
	cairo_stroke(cr);

	a = d = s = r = value();

	cairo_move_to( cr, x_ + 2, y_ + h_ );
	cairo_line_to( cr, x_ + 5 + (w_ * (a / 5.f)), y_ + h_ * 0.1 ); // attack
	cairo_rel_line_to( cr, w_ * (d / 5.2f),   (h_*0.9) * s    ); // decay, and sustain height
	cairo_rel_line_to( cr, w_ * 0.4, 0 );                       // sustain horizontal line
	cairo_rel_line_to( cr, 0.85 * w_ * ( (r) / 5.f), h_ - (h_*0.9) * s - h_ * 0.1  ); // Y down

	cairo_set_line_join( cr, CAIRO_LINE_JOIN_ROUND);
	theme_->color( cr, HIGHLIGHT, 0.2 );
	cairo_fill_preserve( cr );

	theme_->color( cr, HIGHLIGHT, 0.8 );
	cairo_set_line_width(cr, 2.0);
	cairo_stroke(cr);

	cairo_restore( cr );
}

