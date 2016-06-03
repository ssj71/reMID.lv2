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

#include "dialog.hxx"

#include "button.hxx"
#include "ui.hxx"
#include "theme.hxx"

#include <stdio.h>

using namespace Avtk;

Dialog::Dialog( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Group( ui, x_, y_, w_, h_, label_ )
{
	mx = my = -1;

	ok     = new Button( ui, 0, 0, 60, 20, "OK" );
	cancel = new Button( ui, 0, 0, 60, 20, "Cancel" );

	add( ok     );
	add( cancel );

	visible_ = false;

	end();
}

void Dialog::draw( cairo_t* cr )
{
	cairo_save( cr );

	// buttons_
	if( buttons_ == OK ) {
		ok->x( x_ + w_ * 0.8  );
		ok->y( y_ + h_ * 0.75 );
		ok->visible( true );
	} else if( buttons_ == OK_CANCEL ) {
		ok->x( x_ + w_ * 0.8 - 70  );
		ok->y( y_ + h_ * 0.75 );
		cancel->x( x_ + w_ * 0.8  );
		cancel->y( y_ + h_ * 0.75 );
		ok->visible( true );
		cancel->visible( true );
	}

	if( mx != -1 || my != -1 ) {
		// get co-ords of the OK button, and find delta to mx/my
		int xd = mx - (ok->x()+ok->w()/2.f);
		int yd = my - (ok->y()+10);

		// clip to *always* show entire dialog (aka, no "off-window" drawing)
		int diagX = x_ + xd;
		int diagY = y_ + yd;

		// clip top left
		if( diagX < 0 ) diagX = 0;
		if( diagY < 0 ) diagY = 0;

		// clip lower right
		if( diagX + w_ > ui->w() ) diagX = ui->w() - w_;
		if( diagY + h_ > ui->h() ) diagY = ui->h() - h_;

		// set
		x( diagX );
		y( diagY );
	}

	// transparent-out the *entire* UI
	cairo_rectangle( cr, 0, 0, ui->w(), ui->h() );
	//theme_->color( cr, BG_DARK, 0.8 );
	cairo_set_source_rgba( cr, 1, 1, 1, 0.2 );
	cairo_fill_preserve( cr );
	cairo_stroke( cr );

	// draw diagonal lines
	int end = ui->w() + ui->h();
	for(int i = 0; i < end; i += 40 ) {
		cairo_move_to( cr,    -10,  i + 10 );
		cairo_line_to( cr, i + 10,     -10 );
	}
	theme_->color( cr, BG_DARK, 0.2 );
	//cairo_set_source_rgba( cr, 1, 1, 1, 0.2 );
	cairo_set_line_width( cr, 13 );
	cairo_stroke( cr );

	// draw dark BG
	cairo_rectangle( cr, x_+1, y_+1, w_-2, h_-1 );
	theme_->color( cr, BG_DARK, 1 );
	cairo_fill_preserve( cr );
	cairo_set_line_width( cr, 1.0 );
	cairo_stroke( cr );
	theme_->color( cr, BG_DARK, 1 );
	cairo_select_font_face(cr, "impact", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

	// draw header
	cairo_rectangle( cr, x_, y_, w_, 14 );
	theme_->color( cr, HIGHLIGHT, 1.0 );
	cairo_fill( cr );

	/// TODO: get the dialog text amount, wrap based on text extents
	/// and draw buttons in Y position to not interfere with text

	// show the header
	cairo_text_extents_t extents;
	cairo_set_font_size(cr, 10.0);
	cairo_text_extents(cr, label(), &extents);
	cairo_move_to(cr, x_ + 4, y_ + 7 + (extents.height / 2) );
	theme_->color( cr, BG_DARK, 1 );
	cairo_show_text( cr, label() );

	// show the content
	cairo_set_font_size(cr, 12.0);
	cairo_text_extents(cr, contents.c_str(), &extents);
	cairo_move_to(cr, x_ + 4, y_  + 25 + 7 + (extents.height / 2) );
	//theme_->color( cr, BG_DARK, 1 );
	cairo_set_source_rgb( cr, 1,1,1 );
	cairo_show_text( cr, contents.c_str() );


	Group::draw( cr );

	cairo_restore( cr );
}

// to highjack the OK / Cancel buttons events
void Dialog::valueCB( Avtk::Widget* widget)
{
	if( widget == ok ) {
		ui->handleOnly( 0x0 );
		visible( false );
		value( 1.f );

		callback( this, ui );
	} else if( widget == cancel ) {
		ui->handleOnly( 0x0 );
		visible( false );
		value( 0.f );

		callback( this, ui );
	} else {
		//printf("Dialog::widgetValueCB() widget %s\n", widget->label() );
	}
}

void Dialog::run( const char* header, const char* text, BUTTONS b, int mx_, int my_ )
{
	// show the dialog
	label( header );
	contents = text;
	buttons_ = b;

	ok->value( false );
	cancel->value( false );

	visible( true );
	// position to have OK/YES under mouse cursor. When -1, ignore
	mx = mx_;
	my = my_;

	// make UI route all events to here, and wait for the valueCB to be called to
	// release the handleOnly
	ui->handleOnly( this );
}
