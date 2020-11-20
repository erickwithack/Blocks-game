// YOU DEFINITELY NEED TO MODIFY THIS FILE.

#include "ui.hxx"

///
/// VIEW CONSTANTS
///

// Colors are red-green-blue(-alpha), each component
// from 0 to 255.
ge211::Color const ball_color{255, 127, 127};
ge211::Color const paddle_color{255, 255, 127};
ge211::Color const brick_color{100, 100, 100};

///
/// VIEW FUNCTIONS
///

// Data members that are references cannot be initialized by assignment
// in the constructor body and must be initialized in a member
// initializer list.
Ui::Ui(Model& model)
        : model(model)
{ }

ge211::Dims<int>
Ui::initial_window_dimensions() const
{
    return model.config.scene_dims;
}

// Use `Sprite_set::add_sprite(Sprite&, Position)` to add each sprite
// to `sprites`.
void
Ui::draw(ge211::Sprite_set& sprites)
{
    sprites.add_sprite(paddle_sprite, this->model.paddle.top_left());
    sprites.add_sprite(ball_sprite, ge211::Posn<int>(this->model.ball.top_left
    ()));
    for (size_t i= 0; i<this->model.bricks.size(); i++){
        sprites.add_sprite(brick_sprite, this->model.bricks[i].top_left());
    }

}

///
/// CONTROLLER FUNCTIONS
///

// You can get a `Key` representing space with `ge211::Key::code(' ')`.
void
Ui::on_key(ge211::Key key)
{
    if (key == ge211::Key::code('q')) {
        quit();
    }
    if (key == ge211::Key::code((char32_t) *"⎵")) {
        this->model.launch();
    }
}

void
Ui::on_frame(double dt)
{
    model.on_frame(dt);
}

// Makes the ball live via `Model::launch()`.
//
// NOTE: The solution is 1 line of code containing 15 non-whitespace
// characters.
void
Ui::on_mouse_up(ge211::Mouse_button, ge211::Posn<int>)
{
    this->model.ball.live = true;
}

// Informs the model of the mouse position (and thus the desired paddle
// position) via `Model::paddle_to(Position)`.
//
// NOTE: This can be done in 1 line as well.
void
Ui::on_mouse_move(ge211::Posn<int> position)
{
    this->model.paddle_to(position.x);
}
