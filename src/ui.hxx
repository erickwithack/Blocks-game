// IT MIGHT BE OKAY TO MODIFY THIS FILE, BUT YOU PROBABLY DON'T
// WANT TO.
//
// Defines the structure and resources of the user interface.
//

#pragma once

#include "model.hxx"

#include <ge211.hxx>

//
// VIEW CONSTANTS
//

/// These constants are defined in `ui.cpp` but declared here so that we care
/// refer to them when initializing the sprites below.
///
/// The keyword `extern` is what prevents this declaration from being a
/// definition. That is, this declaration informs subsequent code about the
/// existence of these constants without actually defining them here.
extern ge211::Color const ball_color, paddle_color, brick_color;


///
/// MAIN UI STRUCT
///

struct Ui
        : ge211::Abstract_game
{
    //
    // CONSTRUCTOR
    //

    /// Constructs a `Ui` given a reference to the model that stores the actual
    /// state of the game.
    ///
    /// The word `explicit` means that this constructor doesn't define an
    /// implicit conversion whereby you could pass a `Model` to a function that
    /// expects a `Ui` and have it work.
    explicit Ui(Model&);

    //
    // MEMBER FUNCTIONS (for the view)
    //

    /// Returns the size of the window as given by `config_.scene_dims`.
    ///
    /// (GE211 calls this to find out how big to make the window.)
    ge211::Dims<int> initial_window_dimensions() const override;

    /// Renders all the game entities to the screen. In particular,
    /// ball_sprite_ is placed at the ball's bounding box's top-left,
    /// paddle_sprite_ is placed where the model says the paddle is, and
    /// brick_sprite_ is placed for each brick in model_.bricks.
    void draw(ge211::Sprite_set&) override;

    // ball.top_left();
    // MEMBER FUNCTIONS (for the controller)
    //

    /// Defines how the game responds to key events. There are two keys
    /// recognized:
    ///
    ///  - On 'q', quits.
    ///
    ///  - On ' ', tells the model to launch the ball.
    void on_key(ge211::Key) override;

    /// Defines how the game responds to mouse clicks (which is by telling the
    /// model to launch the ball).
    void on_mouse_up(ge211::Mouse_button, ge211::Posn<int>) override;

    /// Defines how the game responds to mouse motion (which is by
    /// telling the model to change the x coordinate of the paddle).
    void on_mouse_move(ge211::Posn<int>) override;

    /// Updates the model to reflect the passage of time, by calling
    /// `Model::on_frame`. This function is passed `dt`, the elapsed time
    /// in seconds since the previous frame.
    void on_frame(double dt) override;

    //
    // MEMBER VARIABLE (model reference)
    //

    /// This is a reference to the model. It means that the UI doesn't
    /// own the model but has access to it. Thus, the client of the UI
    /// (brick_out.cpp) must create a `Model` first and then pass that
    /// by reference to the `Ui` constructor.
    Model& model;

    //
    // MEMBER VARIABLES (for the view)
    //

    // These three definitions create the sprites that we need to
    // display the circle, the paddle, and the bricks to the screen.
    // Each depends on the configuration, which it can access via the
    // `model_` member variable. Member variables are initialized in
    // order, which means that `model_` will be properly initialized
    // before we use it here.

    ge211::Circle_sprite const
            ball_sprite{model.config.ball_radius, ball_color};

    ge211::Rectangle_sprite const
            paddle_sprite{model.config.paddle_dims, paddle_color};

    ge211::Rectangle_sprite const
            brick_sprite{model.config.brick_dims(), brick_color};
};
