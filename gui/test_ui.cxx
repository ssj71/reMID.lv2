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

#include "test_ui.hxx"
#include "avtk/utils.hxx"
#include "avtk/theme.hxx"


#include <sstream>

TestUI::TestUI( PuglNativeWindow parent ):
	Avtk::UI( 600, 480, parent )
{
	Avtk::Widget* w = 0;

    w = new Avtk::Button( this, 10, 10, 20, 40, "Toggle");

/*
	// group testing
	w = new Avtk::Box( this, 610, 43, 100, 125, "Items" );
	group1 = new Avtk::Group( this, 610, 43+16, 100, 0, "Group 1" );
	group1->mode( Avtk::Group::WIDTH_EQUAL );
	group1->valueMode ( Group::VALUE_SINGLE_CHILD );
	group1->resizeMode( Group::RESIZE_FIT_TO_CHILDREN );

	// buttons
	w = new Avtk::Box( this, 610+100+6, 43, 84, 125, "Buttons" );
	group1 = new Avtk::Group( this, 610+100+6+2, 43+16+2, 80, 0, "ButtonsGrp" );
	group1->mode( Avtk::Group::WIDTH_EQUAL );
	group1->spacing( 5 );

	momentary = new Avtk::Button( this, 0, 0, 0, 22, "Button" );
	momentary->theme( theme( 1 ) );
	//momentary->clickMode( Avtk::Widget::CLICK_MOMENTARY );

	w = new Avtk::Button( this, 0, 0, 0, 22, "Toggle" );
	w->theme( theme( 2 ) );
	w->clickMode( Avtk::Widget::CLICK_TOGGLE );


	momentary = new Avtk::Button( this, 0, 0, 0, 22, "Button" );
	momentary->theme( theme( 1 ) );
	//momentary->clickMode( Avtk::Widget::CLICK_MOMENTARY );

	w = new Avtk::Button( this, 0, 0, 0, 22, "Toggle" );
	w->theme( theme( 2 ) );
	w->clickMode( Avtk::Widget::CLICK_TOGGLE );
	group1->end();



	// Editor
	w = new Avtk::Box( this, 130, 43, 120*3, 60*3+14, "MIDI Editor" );
	scroll = new Avtk::Scroll( this, 130, 43+15, 120*3+3, 60*3+6, "Scroll 1" );
	scroll->setCtrlZoom( true );
	int scale = 4;
	editor = new Avtk::EventEditor( this, 0, 0, 240*scale, 250*scale, "EventEditor" );
	editor->value( true );
	editor->visible( true );
	scroll->set( editor );
	scroll->end();

	// dial group
	w = new Avtk::Box( this, 500, 43+130, 210, 68, "Dials" );
	group1 = new Avtk::Group( this, 500, 43+130+ 19, 212, 50, "DialGroup" );
	group1->mode( Avtk::Group::HEIGHT_EQUAL );
	group1->spacing( -2 );

	int dw = 44;
	w = new Avtk::Dial( this, 0, 0, dw, dw, "Dial 1" );
	w = new Avtk::Dial( this, 0, 0, dw, dw, "Dial 2" );
	w = new Avtk::Dial( this, 0, 0, dw, dw, "Dial 3" );
	w = new Avtk::Dial( this, 0, 0, dw, dw, "Dial 4" );
	w = new Avtk::Dial( this, 0, 0, dw, dw, "Dial 5" );

	group1->end();
    */


}

