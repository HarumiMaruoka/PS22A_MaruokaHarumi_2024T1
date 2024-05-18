# include <Siv3D.hpp>

// 前方宣言
class Ball;
class Bricks;
class Paddle;
class Wall;
class Score;
class BallManager;
class GameManager;
class Title;
class Playing;
class GameClear;

// 定数
namespace constants {
	namespace brick {
		/// @brief ブロックのサイズ
		constexpr Size SIZE{ 40, 20 };

		/// @brief ブロックの数　縦
		constexpr int Y_COUNT = 5;

		/// @brief ブロックの数　横
		constexpr int X_COUNT = 20;

		/// @brief 合計ブロック数
		constexpr int MAX = Y_COUNT * X_COUNT;
	}

	namespace ball {
		/// @brief ボールの速さ
		constexpr double SPEED = 480.0;
	}

	namespace paddle {
		/// @brief パドルのサイズ
		constexpr Size SIZE{ 60, 10 };
	}

	namespace reflect {
		/// @brief 縦方向ベクトル
		constexpr Vec2 VERTICAL{ 1, -1 };

		/// @brief 横方向ベクトル
		constexpr Vec2 HORIZONTAL{ -1,  1 };
	}
}

// クラス宣言
class Ball final {
private:
	/// @brief 速度
	Vec2 velocity;

	/// @brief ボール
	Circle ball;

public:
	/// @brief コンストラクタ
	Ball() : velocity({ 0, -constants::ball::SPEED }), ball({ 400, 400, 8 }) {}

	/// @brief デストラクタ
	~Ball() {}

	/// @brief 更新
	void Update() {
		ball.moveBy(velocity * Scene::DeltaTime());
	}

	/// @brief 描画
	void Draw() const {
		ball.draw();
	}

	Circle GetCircle() const {

		return ball;
	}

	Vec2 GetVelocity() const {
		return velocity;
	}

	bool IsDead() const {
		return ball.y > Scene::Height();
	}

	/// @brief 新しい移動速度を設定
	/// @param newVelocity 新しい移動速度
	void SetVelocity(Vec2 newVelocity) {
		using namespace constants::ball;
		velocity = newVelocity.setLength(SPEED);
	}

	/// @brief 反射
	/// @param reflectVec 反射ベクトル方向 
	void Reflect(const Vec2 reflectVec) {
		velocity *= reflectVec;
	}

	void ReStart() {
		velocity = { 0, -constants::ball::SPEED };
		ball = { 400, 400, 8 };
	}
};

class Bricks final {
private:
	/// @brief ブロックリスト
	Rect brickTable[constants::brick::MAX];

public:
	Bricks() {
		using namespace constants::brick;

		for (int y = 0; y < Y_COUNT; ++y) {
			for (int x = 0; x < X_COUNT; ++x) {
				int index = y * X_COUNT + x;
				brickTable[index] = Rect{
					x * SIZE.x,
					60 + y * SIZE.y,
					SIZE
				};
			}
		}
	}

	~Bricks() {}

	bool IsAllDead() const {
		for (int i = 0; i < constants::brick::MAX; ++i) {
			if (brickTable[i].y > 0) {
				return false;
			}
		}
		return true;
	}

	void Intersects(Ball* const target, Score* const score, GameManager* gameManage);

	void Draw() const {
		using namespace constants::brick;

		for (int i = 0; i < MAX; ++i) {
			brickTable[i].stretched(-1).draw(HSV{ brickTable[i].y - 40 });
		}
	}
};

class Paddle final {
private:
	Rect paddle;

public:
	/// @brief コンストラクタ
	Paddle() : paddle(Rect(Arg::center(Cursor::Pos().x, 500), constants::paddle::SIZE)) {}

	/// @brief デストラクタ
	~Paddle() {}

	/// @brief 衝突検知
	void Intersects(Ball* const target) const;

	/// @brief 更新
	void Update() {
		paddle.x = Cursor::Pos().x - (constants::paddle::SIZE.x / 2);
	}

	/// @brief 描画
	void Draw() const {
		paddle.rounded(3).draw();
	}
};

class Wall {
public:
	/// @brief 衝突検知
	static void Intersects(Ball* target) {
		using namespace constants;

		if (!target) {
			return;
		}

		auto velocity = target->GetVelocity();
		auto ball = target->GetCircle();

		// 天井との衝突を検知
		if ((ball.y < 0) && (velocity.y < 0))
		{
			target->Reflect(reflect::VERTICAL);
		}

		// 壁との衝突を検知
		if (((ball.x < 0) && (velocity.x < 0))
			|| ((Scene::Width() < ball.x) && (0 < velocity.x)))
		{
			target->Reflect(reflect::HORIZONTAL);
		}
	}
};

class Score {
public:
	int score;
	Font font;
	Score() : score(0), font(30) {}

	~Score() {}

	void AddScore(int value) {
		score += value;
	}

	void Draw() const {
		font(U"Score: {}"_fmt(score)).draw(20, 0);
	}
};

class BallManager final {
public:
	int ballCount = 3;
	Font deadFont;
	Font ballCountFont;
	BallManager() : deadFont(30), ballCountFont(30) {}
	~BallManager() {}

	void Update(Ball* ball) {
		if (ball->IsDead() && ballCount > 0) {
			ballCount--;
			ball->ReStart();
		}
	}

	void Draw(Ball* const ball, GameManager* gameManager) const;
};

class Title {
public:

	void Start() {
	}

	void Update(GameManager* gameManager);
};

class Playing {
public:
	Playing() {}
	~Playing() {}

	Bricks bricks;
	Ball ball;
	Paddle paddle;
	Score score;
	BallManager ballManager;

	void Start() {
		bricks = Bricks();
		ball = Ball();
		paddle = Paddle();
		score = Score();
		ballManager = BallManager();
	}

	void Update(GameManager* gameManager) {
		paddle.Update();
		ball.Update();
		ballManager.Update(&ball);

		// コリジョン
		bricks.Intersects(&ball, &score, gameManager);
		Wall::Intersects(&ball);
		paddle.Intersects(&ball);

		// 描画
		bricks.Draw();
		ball.Draw();
		paddle.Draw();
		score.Draw();
		ballManager.Draw(&ball, gameManager);
	}
};

class GameClear {
public:
	GameClear() {}
	~GameClear() {}

	void Start() {
	}

	void Update(GameManager* gameManager);
};;

class GameManager {
public:
	GameManager() : title(), playing() {}
	~GameManager() {}

	enum class State {
		TITLE,
		PLAYING,
		GAMECLEAR
	};

	State CurrentState = State::PLAYING;

	Title title;
	Playing playing;
	GameClear gameClear;

	void ChangeState(State state) {
		CurrentState = state;
		switch (state)
		{
		case GameManager::State::TITLE:title.Start(); break;
		case GameManager::State::PLAYING:playing.Start(); break;
		case GameManager::State::GAMECLEAR:gameClear.Start(); break;
		}
	}

	void Update() {
		switch (CurrentState)
		{
		case State::TITLE:title.Update(this); break;
		case State::PLAYING:playing.Update(this); break;
		case State::GAMECLEAR:gameClear.Update(this); break;
		}
	}
};

// 実装
void Bricks::Intersects(Ball* const target, Score* const score, GameManager* gameManage) {
	using namespace constants;
	using namespace constants::brick;

	if (!target) {
		return;
	}

	auto ball = target->GetCircle();

	for (int i = 0; i < MAX; ++i) {
		// 参照で保持
		Rect& refBrick = brickTable[i];

		// 衝突を検知
		if (refBrick.intersects(ball))
		{
			// ブロックの上辺、または底辺と交差
			if (refBrick.bottom().intersects(ball)
				|| refBrick.top().intersects(ball))
			{
				target->Reflect(reflect::VERTICAL);
				score->AddScore(10);
			}
			else // ブロックの左辺または右辺と交差
			{
				target->Reflect(reflect::HORIZONTAL);
				score->AddScore(10);
			}

			// あたったブロックは画面外に出す
			refBrick.y -= Scene::Width() * 2;

			if (IsAllDead()) {
				gameManage->ChangeState(GameManager::State::GAMECLEAR);
			}

			// 同一フレームでは複数のブロック衝突を検知しない
			break;
		}
	}
}

void Paddle::Intersects(Ball* const target) const {
	if (!target) {
		return;
	}

	auto velocity = target->GetVelocity();
	auto ball = target->GetCircle();

	if ((0 < velocity.y) && paddle.intersects(ball))
	{
		target->SetVelocity(Vec2{
			(ball.x - paddle.center().x) * 10,
			-velocity.y
		});
	}
}

void Title::Update(GameManager* gameManager) {

	float buttonWidth = 200;
	if (SimpleGUI::Button(U"Game Start!", { Scene::Width() / 2 - buttonWidth / 2,Scene::Height() / 2 }, buttonWidth)) {
		gameManager->ChangeState(GameManager::State::PLAYING);
	}
}

void BallManager::Draw(Ball* const ball, GameManager* gameManager) const {

	// ボールの残機を表示。
	ballCountFont(U"Ball: {}"_fmt(ballCount)).draw(20, 30);
	// ボールが死んでいたら Dead を表示。
	if (ball->IsDead()) {
		deadFont(U"Dead").drawAt(Scene::Width() / 2, Scene::Height() / 2);

		float buttonWidth = 150;

		if (SimpleGUI::Button(U"ReStart", { Scene::Width() / 2 - buttonWidth / 2, Scene::Height() / 2 + 50 }, buttonWidth)) {
			gameManager->ChangeState(GameManager::State::PLAYING);
		}
		if (SimpleGUI::Button(U"ToTitle", { Scene::Width() / 2 - buttonWidth / 2, Scene::Height() / 2 + 100 }, buttonWidth)) {
			gameManager->ChangeState(GameManager::State::TITLE);
		}
	}
}

void GameClear::Update(GameManager* gameManager) {
	if (SimpleGUI::Button(U"Game Clear!", { Scene::Width() / 2 - 100,Scene::Height() / 2 }, 200)) {
		gameManager->ChangeState(GameManager::State::TITLE);
	}
}

// エントリーポイント
void Main()
{
	GameManager gameManager;
	gameManager.ChangeState(GameManager::State::TITLE);

	while (System::Update())
	{
		gameManager.Update();
	}
}
