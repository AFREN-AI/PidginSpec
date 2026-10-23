{-# LANGUAGE OverloadedStrings #-}

module Scoring
  ( EvaluationCase(..)
  , GoldRequirement(..)
  , Judgment(..)
  , Counts(..)
  , Metrics(..)
  , decodeRows
  , scoreRows
  , calculateMetrics
  , formatSummary
  ) where

import Data.Aeson (FromJSON(..), eitherDecodeStrict', withObject, (.:))
import qualified Data.ByteString as BS
import qualified Data.ByteString.Char8 as BSC
import qualified Data.Map.Strict as Map
import qualified Data.Set as Set
import Data.Text (Text)
import Numeric (showFFloat)

data GoldRequirement = GoldRequirement
  { requirementId :: Text
  , critical :: Bool
  } deriving (Eq, Show)

data Judgment = Judgment
  { predictionId :: Text
  , goldId :: Maybe Text
  , evidenceSupported :: Bool
  } deriving (Eq, Show)

data EvaluationCase = EvaluationCase
  { caseId :: Text
  , goldRequirements :: [GoldRequirement]
  , judgments :: [Judgment]
  } deriving (Eq, Show)

instance FromJSON GoldRequirement where
  parseJSON = withObject "GoldRequirement" $ \obj ->
    GoldRequirement <$> obj .: "id" <*> obj .: "critical"

instance FromJSON Judgment where
  parseJSON = withObject "Judgment" $ \obj ->
    Judgment <$> obj .: "prediction_id" <*> obj .: "gold_id" <*> obj .: "evidence_supported"

instance FromJSON EvaluationCase where
  parseJSON = withObject "EvaluationCase" $ \obj ->
    EvaluationCase <$> obj .: "case_id" <*> obj .: "gold_requirements" <*> obj .: "judgments"

data Counts = Counts
  { casesCount :: Int
  , truePositives :: Int
  , falsePositives :: Int
  , falseNegatives :: Int
  , criticalTruePositives :: Int
  , criticalFalseNegatives :: Int
  } deriving (Eq, Show)

data Metrics = Metrics
  { counts :: Counts
  , precision :: Maybe Double
  , recall :: Maybe Double
  , f1 :: Maybe Double
  , criticalRecall :: Maybe Double
  } deriving (Eq, Show)

decodeRows :: BS.ByteString -> Either String [EvaluationCase]
decodeRows source = traverse decodeLine nonEmptyLines
  where
    nonEmptyLines = filter (not . BS.null . BS.dropWhile isSpaceByte) (BSC.lines source)
    decodeLine line = eitherDecodeStrict' line
    isSpaceByte byte = byte == 32 || byte == 9 || byte == 13

scoreRows :: [EvaluationCase] -> Either String Counts
scoreRows rows = do
  validateRows rows
  pure (foldl' addCase emptyCounts rows)

emptyCounts :: Counts
emptyCounts = Counts 0 0 0 0 0 0

validateRows :: [EvaluationCase] -> Either String ()
validateRows rows
  | hasDuplicates (map caseId rows) = Left "case ids must be unique"
  | otherwise = mapM_ validateCase rows

validateCase :: EvaluationCase -> Either String ()
validateCase row
  | hasDuplicates goldIds = Left ("gold requirement ids must be unique in case " ++ show (caseId row))
  | any (== "") goldIds = Left ("gold requirement ids must be non-empty in case " ++ show (caseId row))
  | hasDuplicates predictionIds = Left ("prediction ids must be unique in case " ++ show (caseId row))
  | any (== "") predictionIds = Left ("prediction ids must be non-empty in case " ++ show (caseId row))
  | caseId row == "" = Left "case_id is required"
  | otherwise = Right ()
  where
    goldIds = map requirementId (goldRequirements row)
    predictionIds = map predictionId (judgments row)

hasDuplicates :: Ord a => [a] -> Bool
hasDuplicates values = Set.size (Set.fromList values) /= length values

addCase :: Counts -> EvaluationCase -> Counts
addCase totals row =
  let gold = Map.fromList [(requirementId item, critical item) | item <- goldRequirements row]
      step (currentTp, currentFp, currentCriticalTp, currentMatched) item =
        case goldId item of
          Just ident
            | evidenceSupported item
            , Just isCritical <- Map.lookup ident gold
            , Set.notMember ident currentMatched ->
                (currentTp + 1, currentFp, currentCriticalTp + if isCritical then 1 else 0, Set.insert ident currentMatched)
          _ -> (currentTp, currentFp + 1, currentCriticalTp, currentMatched)
      (tp, fp, criticalTp, matched) = foldl' step (0, 0, 0, Set.empty) (judgments row)
      missed = [(ident, isCritical) | (ident, isCritical) <- Map.toList gold, Set.notMember ident matched]
      fn = length missed
      criticalFn = length (filter snd missed)
  in totals
      { casesCount = casesCount totals + 1
      , truePositives = truePositives totals + tp
      , falsePositives = falsePositives totals + fp
      , falseNegatives = falseNegatives totals + fn
      , criticalTruePositives = criticalTruePositives totals + criticalTp
      , criticalFalseNegatives = criticalFalseNegatives totals + criticalFn
      }

ratio :: Int -> Int -> Maybe Double
ratio _ 0 = Nothing
ratio numerator denominator = Just (fromIntegral numerator / fromIntegral denominator)

calculateMetrics :: Counts -> Metrics
calculateMetrics values =
  let p = ratio (truePositives values) (truePositives values + falsePositives values)
      r = ratio (truePositives values) (truePositives values + falseNegatives values)
      harmonic = case (p, r) of
        (Just pValue, Just rValue)
          | pValue + rValue > 0 -> Just (2 * pValue * rValue / (pValue + rValue))
        _ -> Nothing
      criticalRate = ratio (criticalTruePositives values) (criticalTruePositives values + criticalFalseNegatives values)
  in Metrics values p r harmonic criticalRate

formatOptional :: Maybe Double -> String
formatOptional Nothing = "null"
formatOptional (Just value) = showFFloat (Just 8) value ""

formatSummary :: Metrics -> String
formatSummary result =
  let values = counts result
  in unlines
      [ "cases\t" ++ show (casesCount values)
      , "true_positives\t" ++ show (truePositives values)
      , "false_positives\t" ++ show (falsePositives values)
      , "false_negatives\t" ++ show (falseNegatives values)
      , "critical_true_positives\t" ++ show (criticalTruePositives values)
      , "critical_false_negatives\t" ++ show (criticalFalseNegatives values)
      , "precision\t" ++ formatOptional (precision result)
      , "recall\t" ++ formatOptional (recall result)
      , "f1\t" ++ formatOptional (f1 result)
      , "critical_recall\t" ++ formatOptional (criticalRecall result)
      ]
