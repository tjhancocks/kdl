test_suite(kdl
        exceptions_test

    # Host
        # Filesystem Path
            file_path_constructWithString
            file_path_constructWithComponents
            file_path_parseComponentsFromString
            file_path_toString_expectedResult
            file_path_isAbsolutePath_startUpVolumeRoot_isTrue
            file_path_isAbsolutePath_homeDirectoryTilde_isTrue
            file_path_isAbsolutePath_siblingFile_isFalse
            file_path_createDirectory
            file_path_directoryExists
            file_path_directoryDoesNotExist
            file_path_directoryName_returnsExpectedResult_whenPathItemIsDirectory
            file_path_directoryName_returnsExpectedResult_whenPathItemIsFile
            file_path_name_returnsExpectedResult_whenDirectory
            file_path_name_returnsExpectedResult_whenNotDirectory
            file_path_fileType_returnsExpectedResult_forFile
            file_path_fileType_returnsExpectedResult_forDirectory
            file_path_fileType_returnsExpectedResult_forNoneExistentFile
            file_path_isDirectory_returnsExpectedResult_forDirectory
            file_path_isDirectory_returnsExpectedResult_forFile
            file_path_childItem_returnsExpectedResult

    # Lexer
        # Analyzer
            lexer_analyze_returnsExpectedSequenceOfLexemes_noErrors
            lexer_analzye_commentsAreIgnored
            lexer_analyze_unrecognisedCharacterExceptionRaised_whenExpected

        # Lexeme
            lexeme_construct_usingTextAndType
            lexeme_construct_usingText_fullInfo
            lexeme_construct_resourceReferenceUsingComponents
            lexeme_components_hasSingleValueMatchingText_whenNoComponents
            lexeme_components_hasExpectedValue_whenSingleComponent
            lexeme_components_hasExpectedValue_whenMultipleComponents
            lexeme_text_hasExpectedValue
            lexeme_text_matchesValueOfCombinedComponents_whenMultipleComponents
            lexeme_isType_matchesCorrectly
            lexeme_isType_doesNotMatchCorrectly
            lexeme_isText_matchesCorrectly
            lexeme_isText_doesNotMatchCorrectly
            lexeme_isTypeAndText_matchesCorrectly
            lexeme_isTypeAndText_doesNotMatchTypeCorrectly
            lexeme_isTypeAndText_doesNotMatchTextCorrectly

        # Decimal Set Condition
            lexerTemplateConditional_decimalSet_containsNumericCharacters
            lexerTemplateConditional_decimalSet_doesNotContainNoneNumericCharacters

        # Hexadecimal Set Condition
            lexerTemplateConditional_hexadecimalSet_containsNumericCharacters
            lexerTemplateConditional_hexadecimalSet_doesNotContainNoneNumericCharacters

        # Identifier Set Condition
            lexerTemplateConditional_identifierSet_completeSet_containsExpectedCharacters
            lexerTemplateConditional_identifierSet_completeSet_doesNotContainUnexpectedCharacters
            lexerTemplateConditional_identifierSet_limitedSet_containsExpectedCharacters
            lexerTemplateConditional_identifierSet_limitedSet_doesNotContainUnexpectedCharacters

        # Match Condition
            lexerTemplateConditional_matchYes_reportsTrueIfCharactersMatch
            lexerTemplateConditional_matchYes_reportsFalseIfCharactersDoNotMatch
            lexerTemplateConditional_matchNo_reportsTrueIfCharactersDoNotMatch
            lexerTemplateConditional_matchNo_reportsFalseIfCharactersMatch

        # Range Condition
            lexerTemplateConditional_rangeContains_reportsTrueIfCharacterIsInRange
            lexerTemplateConditional_rangeNotContains_reportsFalseIfCharacterIsInRange
            lexerTemplateConditional_rangeContains_reportsFalseIfCharacterIsNotInRange
            lexerTemplateConditional_rangeNotContains_reportsTrueIfCharacterIsNotInRange

        # Sequence Condition
            lexerTemplateConditional_sequenceYes_reportsTrueIfStringMatchesTheSequenceOfCharactersDefined
            lexerTemplateConditional_sequenceYes_reportsFalseIfStringDoesNotMatchTheSequenceOfCharactersDefined
            lexerTemplateConditional_sequenceNo_reportsTrueIfStringDoesNotMatchTheSequenceOfCharactersDefined
            lexerTemplateConditional_sequenceNo_reportsFalseIfStringMatchesTheSequenceOfCharactersDefined

        # Set Condition
            lexerTemplateConditional_setContains_trueIfStringConsistsOnlyOfCharactersInSet
            lexerTemplateConditional_setContains_falseIfStringHasCharactersNotInSet
            lexerTemplateConditional_setNotContains_trueIfStringHasCharactersNotInSet
            lexerTemplateConditional_setNotContains_falseIfStringConsistsOnlyOfCharactersInSet

)