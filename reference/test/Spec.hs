module Main (main) where

import Control.Monad (unless)
import qualified Data.Text as Text
import Scoring

main :: IO ()
main = do
  let sample = EvaluationCase
        (Text.pack "case-a")
        [GoldRequirement (Text.pack "r1") True, GoldRequirement (Text.pack "r2") False]
        [ Judgment (Text.pack "p1") (Just (Text.pack "r1")) True
        , Judgment (Text.pack "p2") Nothing False
        ]
      expected = Counts 1 1 1 1 1 0
  result <- pure (scoreRows [sample])
  assertEqual "counts" (Right expected) result
  case result of
    Left message -> fail message
    Right values -> do
      let metrics = calculateMetrics values
      assertEqual "critical recall" (Just 1.0) (criticalRecall metrics)
      assertEqual "F1" (Just (2 / 3)) (f1 metrics)
  let duplicate = sample { goldRequirements =
        [GoldRequirement (Text.pack "same") True, GoldRequirement (Text.pack "same") False] }
  assertLeft "duplicate gold ids" (scoreRows [duplicate])

assertEqual :: (Eq a, Show a) => String -> a -> a -> IO ()
assertEqual label expected actual =
  unless (expected == actual) (fail (label ++ ": expected " ++ show expected ++ ", got " ++ show actual))

assertLeft :: String -> Either String a -> IO ()
assertLeft _ (Left _) = pure ()
assertLeft label (Right _) = fail (label ++ ": expected failure")
